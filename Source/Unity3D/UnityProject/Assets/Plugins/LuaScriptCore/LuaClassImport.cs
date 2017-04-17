using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using AOT;
using System.Reflection;
using System.Collections;
using UnityEngine;

namespace cn.vimfung.luascriptcore.modules.oo
{
	/// <summary>
	/// 类型导入
	/// </summary>
	public class LuaClassImport : LuaModule
	{
		/// <summary>
		/// 实例对象集合
		/// </summary>
		private static List<LuaObjectReference> _instances = new List<LuaObjectReference>();

		/// <summary>
		/// 导出类方法
		/// </summary>
		private static Dictionary<string, Dictionary<string, MethodInfo>> _exportsClassMethods = new Dictionary<string, Dictionary<string, MethodInfo>>();

		/// <summary>
		/// 导出实例方法
		/// </summary>
		private static Dictionary<string, Dictionary<string, MethodInfo>> _exportsInstanceMethods = new Dictionary<string, Dictionary<string, MethodInfo>>();

		/// <summary>
		/// 导出字段Getter
		/// </summary>
		private static Dictionary<string, Dictionary<string, PropertyInfo>> _exportsFieldGetters = new Dictionary<string, Dictionary<string, PropertyInfo>>();

		/// <summary>
		/// 导出字段Setter
		/// </summary>
		private static Dictionary<string, Dictionary<string, PropertyInfo>> _exportsFieldSetters = new Dictionary<string, Dictionary<string, PropertyInfo>>();

		/// <summary>
		/// 导出类型
		/// </summary>
		private static Dictionary<int, Dictionary<string, Type>> _exportsClasses;

		/// <summary>
		/// 是否导出类型委托
		/// </summary>
		private static LuaAllowExportClassDelegate _allowExportClassDelegate;

		/// <summary>
		/// 获取导出类方法委托
		/// </summary>
		private static LuaAllExportClassMethodsDelegte _allExportClassMethodsDelegate;

		/// <summary>
		/// 获取导出实例方法委托
		/// </summary>
		private static LuaAllExportInstanceMethodsDelegte _allExportInstanceMethodsDelegate;

		/// <summary>
		/// 获取导出实例字段Getter委托
		/// </summary>
		private static LuaAllExportInstanceFieldGettersDelegate _allExportInstanceFieldGettersDelegate;

		/// <summary>
		/// 获取导出实例字段Setter委托
		/// </summary>
		private static LuaAllExportInstanceFieldSettersDelegate _allExportInstanceFieldSettersDelegate;

		/// <summary>
		/// 创建原生对象委托
		/// </summary>
		private static LuaCreateNativeObjectDelegate _createNativeObjectDelegate;

		/// <summary>
		/// 类方法调用委托
		/// </summary>
		private static LuaNativeClassMethodHandleDelegate _classMethodInvokeDelegate;

		/// <summary>
		/// 实例方法调用委托
		/// </summary>
		private static LuaNativeInstanceMethodHandleDelegate _instanceMethodInvokeDelegate;

		/// <summary>
		/// 实例字段Getter调用委托
		/// </summary>
		private static LuaNativeInstanceFieldGetterHandleDelegate _fieldGetterInvokeDelegate;

		/// <summary>
		/// 实例字段Setter调用委托
		/// </summary>
		private static LuaNativeInstanceFieldSetterHandleDelegate _fieldSetterInvokeDelegate;

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
			return "ClassImport";
		}

		/// <summary>
		/// 注册模块
		/// </summary>
		/// <param name="context">Lua上下文对象.</param>
		/// <param name="moduleName">模块名称.</param>
		/// <param name="t">类型.</param>
		protected static new void _register(LuaContext context, string moduleName, Type t)
		{

			if (_allowExportClassDelegate == null)
			{
				_allowExportClassDelegate = new LuaAllowExportClassDelegate (_allowExportsClass);
			}

			if (_allExportClassMethodsDelegate == null)
			{
				_allExportClassMethodsDelegate = new LuaAllExportClassMethodsDelegte (_allExportClassMethods);
			}

			if (_allExportInstanceMethodsDelegate == null)
			{
				_allExportInstanceMethodsDelegate = new LuaAllExportInstanceMethodsDelegte (_allExportInstanceMethods);
			}

			if (_allExportInstanceFieldGettersDelegate == null)
			{
				_allExportInstanceFieldGettersDelegate = new LuaAllExportInstanceFieldGettersDelegate (_allExportInstanceFieldGetters);
			}

			if (_allExportInstanceFieldSettersDelegate == null)
			{
				_allExportInstanceFieldSettersDelegate = new LuaAllExportInstanceFieldSettersDelegate (_allExportInstanceFieldSetters);
			}

			if (_createNativeObjectDelegate == null)
			{
				_createNativeObjectDelegate = new LuaCreateNativeObjectDelegate (_createInstance);
			}

			if (_classMethodInvokeDelegate == null)
			{
				_classMethodInvokeDelegate = new LuaNativeClassMethodHandleDelegate (_classMethodInvoke);
			}

			if (_instanceMethodInvokeDelegate == null)
			{
				_instanceMethodInvokeDelegate = new LuaNativeInstanceMethodHandleDelegate (_instanceMethodInvoke);
			}

			if (_fieldGetterInvokeDelegate == null)
			{
				_fieldGetterInvokeDelegate = new LuaNativeInstanceFieldGetterHandleDelegate (_filedGetterInvoke);
			}

			if (_fieldSetterInvokeDelegate == null)
			{
				_fieldSetterInvokeDelegate = new LuaNativeInstanceFieldSetterHandleDelegate (_fieldSetterInvoke);
			}

			NativeUtils.registerClassImport (
				context.objectId, 
				LuaModule.moduleName (t), 
				Marshal.GetFunctionPointerForDelegate(_allowExportClassDelegate), 
				Marshal.GetFunctionPointerForDelegate(_allExportClassMethodsDelegate),
				Marshal.GetFunctionPointerForDelegate(_allExportInstanceMethodsDelegate), 
				Marshal.GetFunctionPointerForDelegate(_allExportInstanceFieldGettersDelegate), 
				Marshal.GetFunctionPointerForDelegate(_allExportInstanceFieldSettersDelegate),
				Marshal.GetFunctionPointerForDelegate(_createNativeObjectDelegate),
				Marshal.GetFunctionPointerForDelegate(_classMethodInvokeDelegate), 
				Marshal.GetFunctionPointerForDelegate(_instanceMethodInvokeDelegate),
				Marshal.GetFunctionPointerForDelegate(_fieldGetterInvokeDelegate), 
				Marshal.GetFunctionPointerForDelegate(_fieldSetterInvokeDelegate));
		}

		/// <summary>
		/// 设置包含导出的类型列表
		/// </summary>
		/// <param name="context">Lua上下文对象.</param>
		/// <param name="classes">导出类型列表.</param>
		public static void setIncludesClasses(LuaContext context, List<Type> classes)
		{
			if (_exportsClasses == null)
			{
				_exportsClasses = new Dictionary<int, Dictionary<string, Type>> ();
			}

			if (context != null)
			{
				Dictionary<string, Type> classesDict = new Dictionary<string, Type> ();
				foreach (Type t in classes)
				{
					classesDict [t.FullName] = t;
				}

				_exportsClasses [context.objectId] = classesDict;
			}
		}

		/// <summary>
		/// 是否允许导出类型
		/// </summary>
		/// <returns>true 表示允许导出，false 表示不允许导出</returns>
		/// <param name="contextId">上下文对象标识</param>
		/// <param name="className">类型名称</param>
		[MonoPInvokeCallback (typeof (LuaAllowExportClassDelegate))]
		private static bool _allowExportsClass(int contextId, string className)
		{
			LuaContext context = LuaContext.getContext (contextId);
			if (context != null)
			{
				if (_exportsClasses.ContainsKey (contextId) && _exportsClasses [contextId] != null)
				{
					return _exportsClasses [contextId].ContainsKey (className);
				}
			}

			return false;
		}

		/// <summary>
		/// 获取所有导出类方法
		/// </summary>
		/// <returns>类方法名称列表.</returns>
		/// <param name="contextId">上下文对象标识</param>
		/// <param name="className">类名.</param>
		[MonoPInvokeCallback (typeof (LuaAllExportClassMethodsDelegte))]
		private static IntPtr _allExportClassMethods(int contextId, string className)
		{
			LuaContext context = LuaContext.getContext (contextId);
			if (context != null)
			{
				HashSet<string> methodNames = new HashSet<string> ();
				Type t =  _exportsClasses [contextId][className];
				if (t != null)
				{
					Dictionary<string, MethodInfo> methods = new Dictionary<string, MethodInfo> ();

					MethodInfo[] methodList = t.GetMethods (BindingFlags.Static | BindingFlags.Public);
					foreach (MethodInfo m in methodList)
					{
						methodNames.Add (m.Name);
						methods [m.Name] = m;
					}

					//放入导出类方法中
					_exportsClassMethods [className] = methods;
				}

				LuaObjectEncoder encoder = new LuaObjectEncoder ();
				encoder.writeInt32 (methodNames.Count);
				foreach (string name in methodNames)
				{
					encoder.writeString (name);
				}

				byte[] bytes = encoder.bytes;
				IntPtr namesPtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, namesPtr, bytes.Length);

				return namesPtr;
			}

			return IntPtr.Zero;
		}
			
		/// <summary>
		/// 获取所有导出实例方法
		/// </summary>
		/// <returns>实例方法名称列表.</returns>
		/// <param name="contextId">上下文对象标识</param>
		/// <param name="className">类名</param>
		[MonoPInvokeCallback (typeof (LuaAllExportInstanceMethodsDelegte))]
		private static IntPtr _allExportInstanceMethods(int contextId, string className)
		{
			LuaContext context = LuaContext.getContext (contextId);
			if (context != null)
			{
				HashSet<string> methodNames = new HashSet<string> ();
				Type t = _exportsClasses [contextId][className];
				if (t != null)
				{
					Dictionary<string, MethodInfo> methods = new Dictionary<string, MethodInfo> ();

					MethodInfo[] methodList = t.GetMethods (BindingFlags.Public | BindingFlags.Instance);
					foreach (MethodInfo m in methodList)
					{
						if (!m.IsStatic)
						{
							methodNames.Add (m.Name);
							methods [m.Name] = m;
						}
					}

					//放入导出类方法中
					_exportsInstanceMethods [className] = methods;
				}

				LuaObjectEncoder encoder = new LuaObjectEncoder ();
				encoder.writeInt32 (methodNames.Count);
				foreach (string name in methodNames)
				{
					encoder.writeString (name);
				}

				byte[] bytes = encoder.bytes;
				IntPtr namesPtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, namesPtr, bytes.Length);

				return namesPtr;
			}

			return IntPtr.Zero;
		}

		/// <summary>
		/// 获取所有导出比例字段Getter
		/// </summary>
		/// <returns>字段Getter名称列表</returns>
		/// <param name="contextId">上下文对象标识.</param>
		/// <param name="className">类名.</param>
		[MonoPInvokeCallback (typeof (LuaAllExportInstanceFieldGettersDelegate))]
		private static IntPtr _allExportInstanceFieldGetters(int contextId, string className)
		{
			LuaContext context = LuaContext.getContext (contextId);
			if (context != null)
			{
				HashSet<string> propNames = new HashSet<string> ();
				Type t = _exportsClasses [contextId][className];
				if (t != null)
				{
					Dictionary<string, PropertyInfo> propertys = new Dictionary<string, PropertyInfo> ();

					PropertyInfo[] propertyList =  t.GetProperties (BindingFlags.Instance | BindingFlags.GetProperty | BindingFlags.Public);
					foreach (PropertyInfo p in propertyList)
					{
						propNames.Add (p.Name);
						propertys [p.Name] = p;
					}

					_exportsFieldGetters [className] = propertys;
				}

				LuaObjectEncoder encoder = new LuaObjectEncoder ();
				encoder.writeInt32 (propNames.Count);
				foreach (string name in propNames)
				{
					encoder.writeString (name);
				}

				byte[] bytes = encoder.bytes;
				IntPtr namesPtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, namesPtr, bytes.Length);

				return namesPtr;

			}

			return IntPtr.Zero;
		}

		/// <summary>
		/// 获取所有导出比例字段Setter
		/// </summary>
		/// <returns>字段Setter名称列表</returns>
		/// <param name="contextId">上下文对象标识.</param>
		/// <param name="className">类名.</param>
		[MonoPInvokeCallback (typeof (LuaAllExportInstanceFieldGettersDelegate))]
		private static IntPtr _allExportInstanceFieldSetters(int contextId, string className)
		{
			LuaContext context = LuaContext.getContext (contextId);
			if (context != null)
			{
				HashSet<string> propNames = new HashSet<string> ();
				Type t = _exportsClasses [contextId][className];
				if (t != null)
				{
					Dictionary<string, PropertyInfo> propertys = new Dictionary<string, PropertyInfo> ();

					PropertyInfo[] propertyList =  t.GetProperties (BindingFlags.SetProperty | BindingFlags.Public | BindingFlags.Instance);
					foreach (PropertyInfo p in propertyList)
					{
						propNames.Add (p.Name);
						propertys [p.Name] = p;
					}

					_exportsFieldSetters [className] = propertys;
				}

				LuaObjectEncoder encoder = new LuaObjectEncoder ();
				encoder.writeInt32 (propNames.Count);
				foreach (string name in propNames)
				{
					encoder.writeString (name);
				}

				byte[] bytes = encoder.bytes;
				IntPtr namesPtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, namesPtr, bytes.Length);

				return namesPtr;

			}

			return IntPtr.Zero;
		}

		/// <summary>
		/// 创建实例对象
		/// </summary>
		/// <returns>实例对象标识.</returns>
		/// <param name="contextId">上下文标识.</param>
		/// <param name="className">类名.</param>
		[MonoPInvokeCallback (typeof (LuaCreateNativeObjectDelegate))]
		private static Int64 _createInstance(int contextId, string className)
		{
			Int64 refId = 0;

			LuaContext context = LuaContext.getContext (contextId);
			if (context != null)
			{
				Type t = _exportsClasses [contextId] [className];
				if (t != null)
				{
					//调用默认构造方法
					ConstructorInfo ci = t.GetConstructor (Type.EmptyTypes);
					if (ci != null)
					{
						object instance = ci.Invoke (null);
						if (instance != null)
						{
							LuaObjectReference objRef = new LuaObjectReference (instance);
							//添加引用避免被GC进行回收
							_instances.Add (objRef);

							refId = objRef.referenceId;
						}
					}
				}
			}

			return refId;
		}

		/// <summary>
		/// 类方法调用
		/// </summary>
		/// <returns>返回值.</returns>
		/// <param name="contextId">上下文标识.</param>
		/// <param name="className">类名称.</param>
		/// <param name="methodName">方法名.</param>
		/// <param name="arguments">参数列表指针.</param>
		/// <param name="size">参数缓冲区大小.</param>
		[MonoPInvokeCallback (typeof (LuaNativeClassMethodHandleDelegate))]
		private static IntPtr _classMethodInvoke(int contextId, string className, string methodName, IntPtr arguments, int size)
		{
			LuaContext context = LuaContext.getContext (contextId);
			if (context != null)
			{
				Type t = _exportsClasses [contextId] [className];
				if (t != null
				   && _exportsClassMethods.ContainsKey (className)
				   && _exportsClassMethods [className].ContainsKey (methodName))
				{
					//存在该方法, 进行调用
					MethodInfo m = _exportsClassMethods [className] [methodName];

					ArrayList argsArr = parseMethodParameters (m, arguments, size);
					object ret = m.Invoke (null, argsArr != null ? argsArr.ToArray () : null);
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
		/// 实例方法调用
		/// </summary>
		/// <returns>返回值.</returns>
		/// <param name="contextId">上下文标识.</param>
		/// <param name="className">类名.</param>
		/// <param name="instance">实例标识.</param>
		/// <param name="methodName">方法名.</param>
		/// <param name="argumentsBuffer">参数列表指针.</param>
		/// <param name="bufferSize">缓存区大小.</param>
		[MonoPInvokeCallback (typeof (LuaNativeInstanceMethodHandleDelegate))]
		private static IntPtr _instanceMethodInvoke(int contextId, string className, Int64 instance, string methodName, IntPtr argumentsBuffer, int bufferSize)
		{
			LuaContext context = LuaContext.getContext (contextId);
			if (context != null)
			{
				Type t = _exportsClasses [contextId] [className];

				if (t != null
				   && instance != 0
				   && _exportsInstanceMethods.ContainsKey (className)
				   && _exportsInstanceMethods [className].ContainsKey (methodName))
				{
					LuaObjectReference objRef = LuaObjectReference.findObject (instance);
					MethodInfo m = _exportsInstanceMethods [className] [methodName];
					if (objRef.target != null && m != null)
					{
						ArrayList argsArr = parseMethodParameters (m, argumentsBuffer, bufferSize);
						object ret = m.Invoke (objRef.target, argsArr != null ? argsArr.ToArray () : null);

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
			}

			return IntPtr.Zero;
		}

		/// <summary>
		/// 字段Getter调用
		/// </summary>
		/// <returns>返回值.</returns>
		/// <param name="contextId">上下文标识.</param>
		/// <param name="className">类名.</param>
		/// <param name="instance">实例标识.</param>
		/// <param name="fieldName">字段名.</param>
		[MonoPInvokeCallback (typeof (LuaNativeInstanceFieldGetterHandleDelegate))]
		private static IntPtr _filedGetterInvoke(int contextId, string className, Int64 instance, string fieldName)
		{
			IntPtr retValuePtr = IntPtr.Zero;

			LuaContext context = LuaContext.getContext (contextId);
			if (context != null)
			{
				Type t = _exportsClasses [contextId] [className];
				if (t != null
				   && instance != 0
				   && _exportsFieldGetters.ContainsKey (className)
				   && _exportsFieldGetters [className].ContainsKey (fieldName))
				{
					LuaObjectReference objRef = LuaObjectReference.findObject (instance);
					PropertyInfo propertyInfo = _exportsFieldGetters [className] [fieldName];
					if (objRef.target != null && propertyInfo != null && propertyInfo.CanRead)
					{
						object retValue = propertyInfo.GetValue (objRef.target, null);
						LuaValue value = new LuaValue (retValue);

						LuaObjectEncoder encoder = new LuaObjectEncoder ();
						encoder.writeObject (value);
						byte[] bytes = encoder.bytes;
						retValuePtr = Marshal.AllocHGlobal (bytes.Length);
						Marshal.Copy (bytes, 0, retValuePtr, bytes.Length);
					}
				}
			}

			return retValuePtr;
		}

		/// <summary>
		/// 字段Setter调用
		/// </summary>
		/// <param name="contextId">上下文标识</param>
		/// <param name="className">类名.</param>
		/// <param name="instance">实例标识.</param>
		/// <param name="fieldName">字段名称.</param>
		/// <param name="valueBuffer">字段值缓存.</param>
		/// <param name="bufferSize">缓存区大小.</param>
		[MonoPInvokeCallback (typeof (LuaNativeInstanceFieldSetterHandleDelegate))]
		private static void _fieldSetterInvoke(int contextId, string className, Int64 instance, string fieldName, IntPtr valueBuffer, int bufferSize)
		{
			Type t = _exportsClasses [contextId][className];

			if (t != null 
				&& instance != 0
				&& _exportsFieldSetters.ContainsKey(className) 
				&& _exportsFieldSetters[className].ContainsKey(fieldName))
			{
				LuaObjectReference objRef = LuaObjectReference.findObject (instance);
				PropertyInfo propertyInfo = _exportsFieldSetters[className][fieldName];
				if (objRef.target != null && propertyInfo != null && propertyInfo.CanWrite)
				{
					LuaObjectDecoder decoder = new LuaObjectDecoder (valueBuffer, bufferSize);
					LuaValue value = decoder.readObject () as LuaValue;

					propertyInfo.SetValue(objRef.target, getNativeValueForLuaValue (propertyInfo.PropertyType, value), null);
				}
			}
		}
	}
}

