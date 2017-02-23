using UnityEngine;
using System.Collections;
using System;
using System.Reflection;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using AOT;

namespace cn.vimfung.luascriptcore.modules.oo
{
	/// <summary>
	/// 面向对象基类
	/// </summary>
	public class LuaObjectClass : LuaModule, ILuaObject
	{
		/// <summary>
		/// 实例对象集合
		/// </summary>
		private static List<LuaObjectClass> _instances = new List<LuaObjectClass>();

		/// <summary>
		/// 导出类型
		/// </summary>
		private static Dictionary<int, Type> _exportsClass = new Dictionary<int, Type>();

		/// <summary>
		/// 导出类方法集合
		/// </summary>
		private static Dictionary<int, Dictionary<string, MethodInfo>> _exportsClassMethods = new Dictionary<int, Dictionary<string, MethodInfo>> (); 

		/// <summary>
		/// 导出实例方法集合
		/// </summary>
		private static Dictionary<int, Dictionary<string, MethodInfo>> _exportsInstanceMethods = new Dictionary<int, Dictionary<string, MethodInfo>> (); 

		/// <summary>
		/// 导出字段集合
		/// </summary>
		private static Dictionary<int, Dictionary<string, PropertyInfo>> _exportsFields = new Dictionary<int, Dictionary<string, PropertyInfo>> (); 

		/// <summary>
		/// 创建实例委托
		/// </summary>
		private static LuaInstanceCreateHandleDelegate _createInstanceDelegate;

		/// <summary>
		/// 销毁实例委托
		/// </summary>
		private static LuaInstanceDestroyHandleDelegate _destroyInstanceDelegate;

		/// <summary>
		/// 实例描述委托
		/// </summary>
		private static LuaInstanceDescriptionHandleDelegate _instanceDescriptionDelegate;

		/// <summary>
		/// 字段获取器委托
		/// </summary>
		private static LuaInstanceFieldGetterHandleDelegate _fieldGetterDelegate;

		/// <summary>
		/// 字段设置器委托
		/// </summary>
		private static LuaInstanceFieldSetterHandleDelegate _fieldSetterDelegate;

		/// <summary>
		/// 实例方法处理器委托
		/// </summary>
		private static LuaInstanceMethodHandleDelegate _instanceMethodHandlerDelegate;

		/// <summary>
		/// 方法处理委托
		/// </summary>
		private static LuaModuleMethodHandleDelegate _classMethodHandleDelegate;

		/// <summary>
		/// 获取模块版本
		/// </summary>
		/// <value>版本号.</value>
		protected static new string _version ()
		{
			return "1.0.0";
		}

		/// <summary>
		/// 获取模块名称
		/// </summary>
		/// <returns>模块名称.</returns>
		protected static new string _moduleName()
		{
			return "Object";
		}

		/// <summary>
		/// 注册模块
		/// </summary>
		/// <param name="context">Lua上下文对象.</param>
		/// <param name="moduleName">模块名称.</param>
		/// <param name="t">类型.</param>
		protected static new void _register(LuaContext context, string moduleName, Type t)
		{
			Type baseType = t.BaseType;

			string superClassName = null;
			if (baseType != typeof(LuaModule))
			{
				superClassName = LuaModule.moduleName (baseType);
				if (!context.isModuleRegisted(superClassName)) 
				{
					//尚未注册父类，进行父类注册
					LuaModule.register(context, baseType);
				}
			}

			//获取导出的类/实例方法
			Dictionary<string, MethodInfo> exportClassMethods = new Dictionary<string, MethodInfo> ();
			List<string> exportClassMethodNames = new List<string> ();

			Dictionary<string, MethodInfo> exportInstanceMethods = new Dictionary<string, MethodInfo> ();
			List<string> exportInstanceMethodNames = new List<string> ();

			MethodInfo[] methods = t.GetMethods();
			foreach (MethodInfo m in methods) 
			{
				if (m.IsStatic && m.IsPublic)
				{
					//静态和公开的方法会导出到Lua
					exportClassMethodNames.Add (m.Name);
					exportClassMethods.Add (m.Name, m);
				} 
				else if (m.IsPublic)
				{
					//实例方法
					exportInstanceMethodNames.Add(m.Name);
					exportInstanceMethods.Add (m.Name, m);
				}
			}

			//获取导出的字段
			Dictionary<string, PropertyInfo> exportFields = new Dictionary<string, PropertyInfo> ();
			List<string> exportSetterNames = new List<string> ();
			List<string> exportGetterNames = new List<string> ();

			PropertyInfo[] propertys = t.GetProperties (BindingFlags.Instance | BindingFlags.Public);
			foreach (PropertyInfo p in propertys) 
			{
				if (p.CanRead)
				{
					exportGetterNames.Add (p.Name);
				}
				if (p.CanWrite)
				{
					exportSetterNames.Add (p.Name);
				}
				exportFields.Add (p.Name, p);
			}

			//创建导出的字段数据
			IntPtr exportSetterNamesPtr = IntPtr.Zero;
			if (exportSetterNames.Count > 0)
			{
				LuaObjectEncoder fieldEncoder = new LuaObjectEncoder ();
				fieldEncoder.writeInt32 (exportSetterNames.Count);
				foreach (string name in exportSetterNames) 
				{
					fieldEncoder.writeString (name);
				}

				byte[] fieldNameBytes = fieldEncoder.bytes;
				exportSetterNamesPtr = Marshal.AllocHGlobal (fieldNameBytes.Length); 
				Marshal.Copy (fieldNameBytes, 0, exportSetterNamesPtr, fieldNameBytes.Length);
			}

			IntPtr exportGetterNamesPtr = IntPtr.Zero;
			if (exportGetterNames.Count > 0)
			{
				LuaObjectEncoder fieldEncoder = new LuaObjectEncoder ();
				fieldEncoder.writeInt32 (exportGetterNames.Count);
				foreach (string name in exportGetterNames) 
				{
					fieldEncoder.writeString (name);
				}

				byte[] fieldNameBytes = fieldEncoder.bytes;
				exportGetterNamesPtr = Marshal.AllocHGlobal (fieldNameBytes.Length); 
				Marshal.Copy (fieldNameBytes, 0, exportGetterNamesPtr, fieldNameBytes.Length);
			}


			//创建导出的实例方法数据
			IntPtr exportInstanceMethodNamesPtr = IntPtr.Zero;
			if (exportInstanceMethodNames.Count > 0)
			{
				LuaObjectEncoder instanceMethodEncoder = new LuaObjectEncoder ();
				instanceMethodEncoder.writeInt32 (exportInstanceMethodNames.Count);
				foreach (string name in exportInstanceMethodNames) 
				{
					instanceMethodEncoder.writeString (name);
				}

				byte[] instMethodNameBytes = instanceMethodEncoder.bytes;
				exportInstanceMethodNamesPtr = Marshal.AllocHGlobal (instMethodNameBytes.Length);
				Marshal.Copy (instMethodNameBytes, 0, exportInstanceMethodNamesPtr, instMethodNameBytes.Length);
			}


			//创建导出类方法数据
			IntPtr exportClassMethodNamesPtr = IntPtr.Zero;
			if (exportClassMethodNames.Count > 0)
			{
				LuaObjectEncoder classMethodEncoder = new LuaObjectEncoder ();
				classMethodEncoder.writeInt32 (exportClassMethodNames.Count);
				foreach (string name in exportClassMethodNames)
				{
					classMethodEncoder.writeString (name);
				}

				byte[] classMethodNameBytes = classMethodEncoder.bytes;
				exportClassMethodNamesPtr = Marshal.AllocHGlobal (classMethodNameBytes.Length);
				Marshal.Copy (classMethodNameBytes, 0, exportClassMethodNamesPtr, classMethodNameBytes.Length);
			}

			//创建实例方法
			if (_createInstanceDelegate == null) 
			{
				_createInstanceDelegate = new LuaInstanceCreateHandleDelegate (_createInstance);
			}

			//销毁实例方法
			if (_destroyInstanceDelegate == null)
			{
				_destroyInstanceDelegate = new LuaInstanceDestroyHandleDelegate (_destroyInstance);
			}

			//类描述
			if (_instanceDescriptionDelegate == null)
			{
				_instanceDescriptionDelegate = new LuaInstanceDescriptionHandleDelegate (_instanceDescription);
			}

			//字段获取器
			if (_fieldGetterDelegate == null) 
			{
				_fieldGetterDelegate = new LuaInstanceFieldGetterHandleDelegate (_fieldGetter);
			}

			//字段设置器
			if (_fieldSetterDelegate == null) 
			{
				_fieldSetterDelegate = new LuaInstanceFieldSetterHandleDelegate (_fieldSetter);
			}

			//实例方法处理器
			if (_instanceMethodHandlerDelegate == null) 
			{
				_instanceMethodHandlerDelegate = new LuaInstanceMethodHandleDelegate (_instanceMethodHandler);
			}

			//类方法处理器
			if (_classMethodHandleDelegate == null)
			{
				_classMethodHandleDelegate = new LuaModuleMethodHandleDelegate (_classMethodHandler);
			}

			int nativeId = NativeUtils.registerClass(
				context.objectId, 
				moduleName, 
				superClassName,
				exportSetterNamesPtr,
				exportGetterNamesPtr,
				exportInstanceMethodNamesPtr,
				exportClassMethodNamesPtr,
				Marshal.GetFunctionPointerForDelegate(_createInstanceDelegate),
				Marshal.GetFunctionPointerForDelegate(_destroyInstanceDelegate),
				Marshal.GetFunctionPointerForDelegate(_instanceDescriptionDelegate),
				Marshal.GetFunctionPointerForDelegate(_fieldGetterDelegate),
				Marshal.GetFunctionPointerForDelegate(_fieldSetterDelegate),
				Marshal.GetFunctionPointerForDelegate(_instanceMethodHandlerDelegate),
				Marshal.GetFunctionPointerForDelegate(_classMethodHandleDelegate));

			//关联注册模块的注册方法
			_exportsClass[nativeId] = t;
			_exportsClassMethods[nativeId] = exportClassMethods;
			_exportsInstanceMethods[nativeId] = exportInstanceMethods;
			_exportsFields[nativeId] = exportFields;
				
			if (exportSetterNamesPtr != IntPtr.Zero)
			{
				Marshal.FreeHGlobal (exportSetterNamesPtr);
			}
			if (exportGetterNamesPtr != IntPtr.Zero)
			{
				Marshal.FreeHGlobal (exportGetterNamesPtr);
			}
			if (exportInstanceMethodNamesPtr != IntPtr.Zero)
			{
				Marshal.FreeHGlobal (exportInstanceMethodNamesPtr);
			}
			if (exportClassMethodNamesPtr != IntPtr.Zero)
			{
				Marshal.FreeHGlobal (exportClassMethodNamesPtr);
			}
		}

		/// <summary>
		/// 序列化对象
		/// </summary>
		/// <param name="encoder">对象编码器.</param>
		public override void serialization (LuaObjectEncoder encoder)
		{
			IntPtr ptr = Marshal.GetIUnknownForObject (this);
			encoder.writeInt64 (ptr.ToInt64 ());
		}

		/// <summary>
		/// 获取对象描述器，用于如何转换成Lua对象
		/// </summary>
		/// <returns>描述器.</returns>
		public LuaObjectDescriptor getDescriptor()
		{
			return new LuaObjectInstanceDescriptor (this);
		}

		/// <summary>
		/// 创建实例对象
		/// </summary>
		/// <param name="nativeClassId">原生类型标识.</param>
		/// <param name="instanceId">实例标识</param>
		[MonoPInvokeCallback (typeof (LuaInstanceCreateHandleDelegate))]
		private static IntPtr _createInstance(int nativeClassId)
		{
			IntPtr instancePtr = IntPtr.Zero;
			Type t = _exportsClass [nativeClassId];
			if (t != null) 
			{
				//调用默认构造方法
				ConstructorInfo ci = t.GetConstructor (Type.EmptyTypes);
				if (ci != null)
				{
					LuaObjectClass instance = ci.Invoke (null) as LuaObjectClass;
					if (instance != null)
					{
						instancePtr = Marshal.GetIUnknownForObject (instance);
						//添加引用避免被GC进行回收
						_instances.Add(instance);
					}
				}
			}

			return instancePtr;
		}

		/// <summary>
		/// 销毁实例对象
		/// </summary>
		/// <param name="nativeClassId">原生类型标识</param>
		/// <param name="instanceId">实例标识</param>
		[MonoPInvokeCallback (typeof (LuaInstanceDestroyHandleDelegate))]
		private static void _destroyInstance(IntPtr instancePtr)
		{
			if (instancePtr != IntPtr.Zero)
			{
				LuaObjectClass instance = Marshal.GetObjectForIUnknown (instancePtr) as LuaObjectClass;
				if (instance != null)
				{
					_instances.Remove (instance);
				}
			}
		}

		/// <summary>
		/// 类型描述
		/// </summary>
		/// <returns>描述信息.</returns>
		/// <param name="nativeClassId">类型标识.</param>
		[MonoPInvokeCallback (typeof (LuaInstanceDescriptionHandleDelegate))]
		private static string _instanceDescription (IntPtr instancePtr)
		{
			if (instancePtr != IntPtr.Zero)
			{
				LuaObjectClass instance = Marshal.GetObjectForIUnknown (instancePtr) as LuaObjectClass;
				if (instance != null)
				{
					return instance.ToString ();
				}
			}

			return "";
		}

		/// <summary>
		/// 字段获取器
		/// </summary>
		/// <returns>字段返回值.</returns>
		/// <param name="classId">类标识.</param>
		/// <param name="instance">实例.</param>
		/// <param name="fieldName">字段名称.</param>
		[MonoPInvokeCallback (typeof (LuaInstanceFieldGetterHandleDelegate))]
		private static IntPtr _fieldGetter (int classId, IntPtr instancePtr, string fieldName)
		{
			IntPtr retValuePtr = IntPtr.Zero;
			if (instancePtr != IntPtr.Zero 
				&& _exportsFields.ContainsKey(classId) 
				&& _exportsFields[classId].ContainsKey(fieldName))
			{
				LuaObjectClass instance = Marshal.GetObjectForIUnknown (instancePtr) as LuaObjectClass;
				PropertyInfo propertyInfo = _exportsFields[classId][fieldName];
				if (instance != null && propertyInfo != null && propertyInfo.CanRead)
				{
					object retValue = propertyInfo.GetValue (instance, null);
					LuaValue value = new LuaValue (retValue);

					LuaObjectEncoder encoder = new LuaObjectEncoder ();
					encoder.writeObject (value);
					byte[] bytes = encoder.bytes;
					retValuePtr = Marshal.AllocHGlobal (bytes.Length);
					Marshal.Copy (bytes, 0, retValuePtr, bytes.Length);
				}
			}

			return retValuePtr;
		}

		/// <summary>
		/// 字段设置器
		/// </summary>
		/// <param name="classId">类标识.</param>
		/// <param name="instance">实例</param>
		/// <param name="fieldName">字段名称</param>
		/// <param name="valueBuffer">值数据</param>
		/// <param name="bufferSize">数据大小</param>
		[MonoPInvokeCallback (typeof (LuaInstanceFieldSetterHandleDelegate))]
		private static void _fieldSetter (int classId, IntPtr instancePtr, string fieldName, IntPtr valueBuffer, int bufferSize)
		{
			if (instancePtr != IntPtr.Zero
				&& _exportsFields.ContainsKey(classId) 
				&& _exportsFields[classId].ContainsKey(fieldName))
			{
				LuaObjectClass instance = Marshal.GetObjectForIUnknown (instancePtr) as LuaObjectClass;
				PropertyInfo propertyInfo = _exportsFields[classId][fieldName];
				if (instance != null && propertyInfo != null && propertyInfo.CanWrite)
				{
					LuaObjectDecoder decoder = new LuaObjectDecoder (valueBuffer, bufferSize);
					LuaValue value = decoder.readObject () as LuaValue;

					propertyInfo.SetValue(instance, getNativeValueForLuaValue (propertyInfo.PropertyType, value), null);
				}
			}
		}

		/// <summary>
		/// 实例方法处理器
		/// </summary>
		/// <returns>返回值</returns>
		/// <param name="classId">类标识</param>
		/// <param name="instance">实例</param>
		/// <param name="methodName">方法名称</param>
		/// <param name="argumentsBuffer">参数数据</param>
		/// <param name="bufferSize">数据大小.</param>
		[MonoPInvokeCallback (typeof (LuaInstanceMethodHandleDelegate))]
		private static IntPtr _instanceMethodHandler (int classId, IntPtr instancePtr, string methodName, IntPtr argumentsBuffer, int bufferSize)
		{
			if (instancePtr != IntPtr.Zero
				&& _exportsInstanceMethods.ContainsKey(classId)
				&& _exportsInstanceMethods[classId].ContainsKey(methodName))
			{
				LuaObjectClass instance = Marshal.GetObjectForIUnknown (instancePtr) as LuaObjectClass;
				MethodInfo m = _exportsInstanceMethods[classId][methodName];
				if (instance != null && m != null)
				{
					ArrayList argsArr = parseMethodParameters (m, argumentsBuffer, bufferSize);
					object ret = m.Invoke (instance, argsArr != null ? argsArr.ToArray() : null);

					LuaValue retValue = new LuaValue (ret);

					LuaObjectEncoder encoder = new LuaObjectEncoder ();
					encoder.writeObject (retValue);

					byte[] bytes = encoder.bytes;
					IntPtr retPtr;
					retPtr = Marshal.AllocHGlobal (bytes.Length);
					Marshal.Copy (bytes, 0, retPtr, bytes.Length);

					return retPtr;
				}

			}
			return IntPtr.Zero;
		}

		/// <summary>
		/// Lua方法路由
		/// </summary>
		/// <returns>返回值</returns>
		/// <param name="classId">类标识</param>
		/// <param name="methodName">方法名称.</param>
		/// <param name="arguments">参数列表缓冲区.</param>
		/// <param name="size">参数列表缓冲区大小.</param>
		[MonoPInvokeCallback (typeof (LuaModuleMethodHandleDelegate))]
		private static IntPtr _classMethodHandler (int classId, string methodName, IntPtr arguments, int size)
		{
			if (_exportsClassMethods.ContainsKey (classId) && _exportsClassMethods[classId].ContainsKey(methodName)) 
			{
				//存在该方法, 进行调用
				MethodInfo m = _exportsClassMethods [classId][methodName];

				ArrayList argsArr = parseMethodParameters (m, arguments, size);
				object ret = m.Invoke (null, argsArr != null ? argsArr.ToArray() : null);
				LuaValue retValue = new LuaValue (ret);

				LuaObjectEncoder encoder = new LuaObjectEncoder ();
				encoder.writeObject (retValue);

				byte[] bytes = encoder.bytes;
				IntPtr retPtr;
				retPtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, retPtr, bytes.Length);

				return retPtr;
			}

			return IntPtr.Zero;
		}
	}
}