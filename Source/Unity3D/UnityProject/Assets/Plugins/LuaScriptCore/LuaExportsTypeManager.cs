using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using AOT;
using System.Collections;
using System.Text;
using UnityEngine;

namespace cn.vimfung.luascriptcore
{
	/// <summary>
	/// 导出类型管理器
	/// </summary>
	internal class LuaExportsTypeManager
	{
		
		/// <summary>
		/// 基础数据类型映射表
		/// </summary>
		static private Dictionary<Type, string> baseTypesMapping = new Dictionary<Type, string> () {
			{ typeof(Char) , 	"c" },
			{ typeof(Int16) , 	"s" },
			{ typeof(UInt16) , 	"S" },
			{ typeof(Int32) , 	"i" },
			{ typeof(UInt32) , 	"I" },
			{ typeof(Int64) , 	"q" },
			{ typeof(UInt64) , 	"Q" },
			{ typeof(Boolean) , "B" },
		}; 

		/// <summary>
		/// 实例对象集合
		/// </summary>
		private static List<LuaObjectReference> _instances = new List<LuaObjectReference>();

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
		private static Dictionary<int, Dictionary<string, PropertyInfo>> _exportsProperties = new Dictionary<int, Dictionary<string, PropertyInfo>> (); 

		/// <summary>
		/// 导出字段集合
		/// </summary>
		private static Dictionary<int, Dictionary<string, FieldInfo>> _exportsFields = new Dictionary<int, Dictionary<string, FieldInfo>> ();

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
		/// 上下文对象
		/// </summary>
		private WeakReference _weakContext;

		/// <summary>
		/// 初始化
		/// </summary>
		/// <param name="context">上下文对象.</param>
		public LuaExportsTypeManager(LuaContext context)
		{
			_weakContext = new WeakReference (context);
		}

		/// <summary>
		/// 获取上下文对象
		/// </summary>
		/// <value>上下文对象.</value>
		public LuaContext context
		{
			get
			{
				return _weakContext.Target as LuaContext;
			}
		}

		/// <summary>
		/// 导出类型
		/// </summary>
		/// <param name="type">类型.</param>
		/// <param name="context">上下文对象.</param>
		public void exportType(Type t, LuaContext context)
		{
			LuaExportTypeAnnotation typeAnnotation = Attribute.GetCustomAttribute (t, typeof(LuaExportTypeAnnotation), false) as LuaExportTypeAnnotation;

			//获取导出的类/实例方法
			Dictionary<string, MethodInfo> exportClassMethods = new Dictionary<string, MethodInfo> ();
			List<string> exportClassMethodNames = new List<string> ();

			Dictionary<string, MethodInfo> exportInstanceMethods = new Dictionary<string, MethodInfo> ();
			List<string> exportInstanceMethodNames = new List<string> ();

			MethodInfo[] methods = t.GetMethods();
			foreach (MethodInfo m in methods) 
			{
				if (m.IsPublic || (m.IsStatic && m.IsPublic))
				{
					StringBuilder methodSignature = new StringBuilder();
					foreach (ParameterInfo paramInfo in m.GetParameters())
					{
						Type paramType = paramInfo.ParameterType;
						if (baseTypesMapping.ContainsKey (paramType))
						{
							methodSignature.Append (baseTypesMapping [paramType]);
						}
						else
						{
							methodSignature.Append ("@");
						}
					}

					string methodName = string.Format ("{0}_{1}", m.Name, methodSignature.ToString ());

					LuaExclude isExclude = Attribute.GetCustomAttribute (m, typeof(LuaExclude), true) as LuaExclude;
					if (isExclude != null)
					{
						//为过滤方法
						continue;
					}

					if (m.IsStatic && m.IsPublic)
					{
						if (typeAnnotation != null
							&& typeAnnotation.excludeExportClassMethodNames != null 
							&& Array.IndexOf (typeAnnotation.excludeExportClassMethodNames, m.Name) >= 0)
						{
							//为过滤方法
							continue;
						}

						//静态和公开的方法会导出到Lua
						exportClassMethodNames.Add (methodName);
						exportClassMethods.Add (methodName, m);
					} 
					else if (m.IsPublic)
					{
						if (typeAnnotation != null &&
							typeAnnotation.excludeExportInstanceMethodNames != null 
							&& Array.IndexOf (typeAnnotation.excludeExportInstanceMethodNames, m.Name) >= 0)
						{
							//为过滤方法
							continue;
						}

						//实例方法
						exportInstanceMethodNames.Add(methodName);
						exportInstanceMethods.Add (methodName, m);
					}
				}
			}

			//获取导出的字段
			Dictionary<string, FieldInfo> exportFields = new Dictionary<string, FieldInfo> ();
			List<string> exportPropertyNames = new List<string> ();

			FieldInfo[] fields = t.GetFields (BindingFlags.Instance | BindingFlags.Public);
			foreach (FieldInfo f in fields)
			{
				LuaExclude isExclude = Attribute.GetCustomAttribute (f, typeof(LuaExclude), true) as LuaExclude;
				if (isExclude != null)
				{
					//为过滤属性
					continue;
				}

				if (typeAnnotation != null 
					&& typeAnnotation.excludeExportPropertyNames != null 
					&& Array.IndexOf (typeAnnotation.excludeExportPropertyNames, f.Name) >= 0)
				{
					//在过滤列表中
					continue;
				}

				exportPropertyNames.Add(string.Format("{0}_rw", f.Name));
				exportFields.Add (f.Name, f);
			}

			//获取导出的属性
			Dictionary<string, PropertyInfo> exportProperties = new Dictionary<string, PropertyInfo> ();

			PropertyInfo[] propertys = t.GetProperties (BindingFlags.Instance | BindingFlags.Public);
			foreach (PropertyInfo p in propertys) 
			{
				LuaExclude isExclude = Attribute.GetCustomAttribute (p, typeof(LuaExclude), true) as LuaExclude;
				if (isExclude != null)
				{
					//为过滤属性
					continue;
				}

				if (typeAnnotation != null 
					&& typeAnnotation.excludeExportPropertyNames != null 
					&& Array.IndexOf (typeAnnotation.excludeExportPropertyNames, p.Name) >= 0)
				{
					//在过滤列表中
					continue;
				}

				StringBuilder actStringBuilder = new StringBuilder ();
				if (p.CanRead)
				{
					actStringBuilder.Append ("r");
				}
				if (p.CanWrite)
				{
					actStringBuilder.Append ("w");
				}

				exportPropertyNames.Add(string.Format("{0}_{1}", p.Name, actStringBuilder.ToString()));
				exportProperties.Add (p.Name, p);
			}

			//创建导出的字段数据
			IntPtr exportPropertyNamesPtr = IntPtr.Zero;
			if (exportPropertyNames.Count > 0)
			{
				LuaObjectEncoder fieldEncoder = new LuaObjectEncoder (context);
				fieldEncoder.writeInt32 (exportPropertyNames.Count);
				foreach (string name in exportPropertyNames) 
				{
					fieldEncoder.writeString (name);
				}

				byte[] fieldNameBytes = fieldEncoder.bytes;
				exportPropertyNamesPtr = Marshal.AllocHGlobal (fieldNameBytes.Length); 
				Marshal.Copy (fieldNameBytes, 0, exportPropertyNamesPtr, fieldNameBytes.Length);
			}


			//创建导出的实例方法数据
			IntPtr exportInstanceMethodNamesPtr = IntPtr.Zero;
			if (exportInstanceMethodNames.Count > 0)
			{
				LuaObjectEncoder instanceMethodEncoder = new LuaObjectEncoder (context);
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
				LuaObjectEncoder classMethodEncoder = new LuaObjectEncoder (context);
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
				
			int typeId = NativeUtils.registerType (
				context.objectId, 
				t.Name,
				t.FullName,
				t.BaseType.FullName,
				exportPropertyNamesPtr,
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
			_exportsClass[typeId] = t;
			_exportsClassMethods[typeId] = exportClassMethods;
			_exportsInstanceMethods[typeId] = exportInstanceMethods;
			_exportsProperties[typeId] = exportProperties;
			_exportsFields [typeId] = exportFields;

			if (exportPropertyNamesPtr != IntPtr.Zero)
			{
				Marshal.FreeHGlobal (exportPropertyNamesPtr);
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
		/// 创建实例对象
		/// </summary>
		/// <param name="nativeClassId">原生类型标识.</param>
		/// <param name="instanceId">实例标识</param>
		[MonoPInvokeCallback (typeof (LuaInstanceCreateHandleDelegate))]
		private static Int64 _createInstance(int contextId, int nativeClassId, IntPtr argumentsBuffer, int bufferSize)
		{
			Int64 refId = -1;

			if (_exportsClass.ContainsKey (nativeClassId))
			{
				Type t = _exportsClass [nativeClassId];
				if (t != null) 
				{
					LuaContext context = LuaContext.getContext (contextId);
					List<LuaValue> arguments = getArgumentList (context, argumentsBuffer, bufferSize);

					ConstructorInfo ci = getConstructor (t, t, arguments);
					if (ci != null)
					{
						ArrayList argsArr = new ArrayList ();
						ParameterInfo[] parameters = ci.GetParameters ();
						if (parameters.Length > 0 && arguments != null) 
						{
							int i = 0;
							foreach (ParameterInfo p in parameters) 
							{
								if (i >= arguments.Count) 
								{
									break;
								}

								object value = getNativeValueForLuaValue(p.ParameterType, arguments[i]);
								argsArr.Add (value);

								i++;
							}
						}

						object instance = ci.Invoke (argsArr.ToArray ());
						if (instance != null)
						{
							LuaObjectReference objRef = new LuaObjectReference (instance);
							//添加引用避免被GC进行回收
							_instances.Add(objRef);

							refId = objRef.referenceId;
						}
					}
				}
			}

			return refId;
		}

		/// <summary>
		/// 销毁实例对象
		/// </summary>
		/// <param name="nativeClassId">原生类型标识</param>
		/// <param name="instanceId">实例标识</param>
		[MonoPInvokeCallback (typeof (LuaInstanceDestroyHandleDelegate))]
		private static void _destroyInstance(Int64 instancePtr)
		{
			if (instancePtr != 0)
			{
				LuaObjectReference instanceRef = LuaObjectReference.findObject (instancePtr);
				if (instanceRef != null)
				{
					_instances.Remove (instanceRef);
				}
			}
		}

		/// <summary>
		/// 类型描述
		/// </summary>
		/// <returns>描述信息.</returns>
		/// <param name="nativeClassId">类型标识.</param>
		[MonoPInvokeCallback (typeof (LuaInstanceDescriptionHandleDelegate))]
		private static string _instanceDescription (Int64 instancePtr)
		{
			if (instancePtr != 0)
			{
				LuaObjectReference objRef = LuaObjectReference.findObject (instancePtr);
				if (objRef != null)
				{
					return objRef.target.ToString ();
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
		private static IntPtr _fieldGetter (int contextId, int classId, Int64 instancePtr, string fieldName)
		{
			IntPtr retValuePtr = IntPtr.Zero;
			if (instancePtr != 0)
			{
				LuaContext context = LuaContext.getContext (contextId);
				LuaObjectReference objRef = LuaObjectReference.findObject (instancePtr);
				object instance = objRef.target;

				LuaValue value = null;

				if (_exportsProperties.ContainsKey (classId)
				    && _exportsProperties [classId].ContainsKey (fieldName))
				{
					PropertyInfo propertyInfo = _exportsProperties [classId] [fieldName];
					if (instance != null && propertyInfo != null && propertyInfo.CanRead)
					{
						object retValue = propertyInfo.GetValue (instance, null);
						value = new LuaValue (retValue);
					}
				}
				else if (_exportsFields.ContainsKey (classId)
				         && _exportsFields [classId].ContainsKey (fieldName))
				{
					FieldInfo fieldInfo = _exportsFields [classId] [fieldName];
					if (instance != null && fieldInfo != null)
					{
						object retValue = fieldInfo.GetValue (instance);
						value = new LuaValue (retValue);
					}
				}

				if (value != null)
				{
					LuaObjectEncoder encoder = new LuaObjectEncoder (context);
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
		private static void _fieldSetter (int contextId, int classId, Int64 instancePtr, string fieldName, IntPtr valueBuffer, int bufferSize)
		{
			if (instancePtr != 0)
			{
				
				LuaContext context = LuaContext.getContext (contextId);
				LuaObjectReference objRef = LuaObjectReference.findObject (instancePtr);
				object instance = objRef.target;
				LuaObjectDecoder decoder = new LuaObjectDecoder (valueBuffer, bufferSize, context);
				LuaValue value = decoder.readObject () as LuaValue;


				if (_exportsProperties.ContainsKey (classId)
				    && _exportsProperties [classId].ContainsKey (fieldName))
				{
					PropertyInfo propertyInfo = _exportsProperties [classId] [fieldName];
					if (instance != null && propertyInfo != null && propertyInfo.CanWrite)
					{
						propertyInfo.SetValue (instance, getNativeValueForLuaValue (propertyInfo.PropertyType, value), null);
					}
				}
				else if (_exportsFields.ContainsKey (classId)
				         && _exportsFields [classId].ContainsKey (fieldName))
				{
					FieldInfo fieldInfo = _exportsFields [classId] [fieldName];
					if (instance != null && fieldInfo != null)
					{
						fieldInfo.SetValue (instance, getNativeValueForLuaValue (fieldInfo.FieldType, value));
					}
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
		private static IntPtr _instanceMethodHandler (int contextId, int classId, Int64 instancePtr, string methodName, IntPtr argumentsBuffer, int bufferSize)
		{
			if (instancePtr != 0
				&& _exportsInstanceMethods.ContainsKey(classId)
				&& _exportsInstanceMethods[classId].ContainsKey(methodName))
			{
				LuaContext context = LuaContext.getContext (contextId);
				LuaObjectReference objRef = LuaObjectReference.findObject (instancePtr);
				object instance = objRef.target;
				MethodInfo m = _exportsInstanceMethods[classId][methodName];
				if (instance != null && m != null)
				{
					ArrayList argsArr = parseMethodParameters (m, getArgumentList(context, argumentsBuffer, bufferSize));
					object ret = m.Invoke (instance, argsArr != null && argsArr.Count > 0  ? argsArr.ToArray() : null);

					LuaValue retValue = new LuaValue (ret);

					LuaObjectEncoder encoder = new LuaObjectEncoder (context);
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
		private static IntPtr _classMethodHandler (int contextId, int classId, string methodName, IntPtr arguments, int size)
		{
			if (_exportsClassMethods.ContainsKey (classId) && _exportsClassMethods[classId].ContainsKey(methodName)) 
			{
				LuaContext context = LuaContext.getContext (contextId);
				//存在该方法, 进行调用
				MethodInfo m = _exportsClassMethods [classId][methodName];

				ArrayList argsArr = parseMethodParameters (m, getArgumentList(context, arguments, size));
				object ret = m.Invoke (null, argsArr != null ? argsArr.ToArray() : null);
				LuaValue retValue = new LuaValue (ret);

				LuaObjectEncoder encoder = new LuaObjectEncoder (context);
				encoder.writeObject (retValue);

				byte[] bytes = encoder.bytes;
				IntPtr retPtr;
				retPtr = Marshal.AllocHGlobal (bytes.Length);
				Marshal.Copy (bytes, 0, retPtr, bytes.Length);

				return retPtr;
			}

			return IntPtr.Zero;
		}

		/// <summary>
		/// 获取类型匹配的构造方法
		/// </summary>
		/// <returns>构造方法.</returns>
		/// <param name="t">类型.</param>
		/// <param name="arguments">参数列表.</param>
		protected static ConstructorInfo getConstructor(Type targetType, Type t, List<LuaValue> arguments)
		{
			int targetMatchDegree = 0;
			ConstructorInfo targetConstructor = null;
			ConstructorInfo defaultConstructor = null;

			if (t.GetInterface("cn.vimfung.luascriptcore.LuaExportType") != null)
			{
				ConstructorInfo[] constructors = t.GetConstructors ();
				foreach (ConstructorInfo constructor in constructors)
				{
					//检测是否为排除方法
					LuaExclude isExclude = Attribute.GetCustomAttribute (constructor, typeof(LuaExclude), true) as LuaExclude;
					if (isExclude != null)
					{
						continue;
					}

					int matchDegree = 0;
					int index = 0;

					ParameterInfo[] paramInfos = constructor.GetParameters ();
					if (paramInfos.Length == 0)
					{
						//默认构造函数
						defaultConstructor = constructor;
					}

					if (paramInfos.Length != arguments.Count)
					{
						continue;
					}

					bool hasMatch = true;
					foreach (ParameterInfo paramInfo in paramInfos)
					{
						Type paramType = paramInfo.ParameterType;
						LuaValue arg = arguments [index];

						if (typeof(int) == paramType
						    || typeof(long) == paramType
						    || typeof(short) == paramType
						    || typeof(uint) == paramType
						    || typeof(ushort) == paramType
						    || typeof(ulong) == paramType)
						{
							if (arg.type == LuaValueType.Integer || arg.type == LuaValueType.Number)
							{
								matchDegree++;
							}
						}
						else if (typeof(float) == paramType
						         || typeof(double) == paramType)
						{
							if (arg.type == LuaValueType.Number)
							{
								matchDegree++;
							}
						}
						else if (typeof(bool) == paramType)
						{
							if (arg.type == LuaValueType.Boolean)
							{
								matchDegree++;
							}
						}
						else if (typeof(string) == paramType)
						{
							if (arg.type == LuaValueType.String)
							{
								matchDegree++;
							}
						}
						else if (typeof(byte[]) == paramType)
						{
							if (arg.type == LuaValueType.Data
							    || arg.type == LuaValueType.String)
							{
								matchDegree++;
							}
							else
							{
								hasMatch = false;
							}
						}
						else if (paramType.IsArray || typeof(IList).IsAssignableFrom (paramType))
						{
							if (arg.type == LuaValueType.Array)
							{
								matchDegree++;
							}
							else
							{
								hasMatch = false;
							}
						}
						else if (typeof(IDictionary).IsAssignableFrom (t))
						{
							if (arg.type == LuaValueType.Map)
							{
								matchDegree++;
							}
							else
							{
								hasMatch = false;
							}
						}
						else
						{
							object obj = arg.toObject ();
							if (obj.GetType ().IsAssignableFrom (paramType))
							{
								matchDegree++;
							}
							else
							{
								hasMatch = false;
							}
						}

						if (!hasMatch)
						{
							break;
						}

						index++;
					}

					if (hasMatch && matchDegree > targetMatchDegree)
					{
						targetConstructor = constructor;
						targetMatchDegree = matchDegree;
					}
				}

				if (targetConstructor == null)
				{
					//检测父类构造方法
					targetConstructor = getConstructor(targetType, t.BaseType, arguments);
				}

				if (targetConstructor == null && targetType == t)
				{
					targetConstructor = defaultConstructor;
				}
					
			}

			return targetConstructor;
		}

		protected static object getNativeValueForLuaValue(Type t, LuaValue luaValue)
		{
			object value = null;
			if (t == typeof(Int32)
				|| t == typeof(Int64)
				|| t == typeof(Int16)
				|| t == typeof(UInt16)
				|| t == typeof(UInt32)
				|| t == typeof(UInt64)) 
			{
				value = luaValue.toInteger();
			} 
			else if (t == typeof(double)
				|| t == typeof(float)) 
			{
				value = luaValue.toNumber();
			} 
			else if (t == typeof(bool))
			{
				value = luaValue.toBoolean();
			}
			else if (t == typeof(string)) 
			{
				value = luaValue.toString();
			}
			else if (t.IsArray || typeof(IList).IsAssignableFrom(t)) 
			{
				//数组
				if (t == typeof(byte[])) 
				{
					//二进制数组
					value = luaValue.toData();
				}
				else if (t == typeof(Int32[]))
				{
					List<LuaValue> arr = luaValue.toArray();
					if (arr != null) 
					{
						List<Int32> intArr = new List<Int32> ();
						foreach (LuaValue item in arr) 
						{
							intArr.Add (item.toInteger ());
						}
						value = intArr.ToArray ();
					}
				}
				else if (t == typeof(Int64[]))
				{
					List<LuaValue> arr = luaValue.toArray();
					if (arr != null) 
					{
						List<Int64> intArr = new List<Int64> ();
						foreach (LuaValue item in arr) 
						{
							intArr.Add (item.toInteger ());
						}
						value = intArr.ToArray ();
					}
				}
				else if (t == typeof(Int16[]))
				{
					List<LuaValue> arr = luaValue.toArray();
					if (arr != null) 
					{
						List<Int16> intArr = new List<Int16> ();
						foreach (LuaValue item in arr) 
						{
							intArr.Add (Convert.ToInt16 (item.toInteger ()));
						}
						value = intArr.ToArray ();
					}
				}
				else if (t == typeof(UInt32[]))
				{
					List<LuaValue> arr = luaValue.toArray();
					if (arr != null) 
					{
						List<UInt32> intArr = new List<UInt32> ();
						foreach (LuaValue item in arr) 
						{
							intArr.Add (Convert.ToUInt32 (item.toInteger ()));
						}
						value = intArr.ToArray ();
					}

				}
				else if (t == typeof(UInt64[]))
				{
					List<LuaValue> arr = luaValue.toArray();
					if (arr != null) 
					{
						List<UInt64> intArr = new List<UInt64> ();
						foreach (LuaValue item in arr) 
						{
							intArr.Add (Convert.ToUInt64 (item.toInteger ()));
						}
						value = intArr.ToArray ();
					}
				}
				else if (t == typeof(UInt16[]))
				{
					List<LuaValue> arr = luaValue.toArray();
					if (arr != null) 
					{
						List<UInt16> intArr = new List<UInt16> ();
						foreach (LuaValue item in arr) 
						{
							intArr.Add (Convert.ToUInt16 (item.toInteger ()));
						}
						value = intArr.ToArray ();
					}
				}
				else if (t == typeof(bool[]))
				{
					List<LuaValue> arr = luaValue.toArray();
					if (arr != null)
					{
						List<bool> boolArr = new List<bool> ();
						foreach (LuaValue item in arr) 
						{
							boolArr.Add (Convert.ToBoolean (item.toInteger ()));
						}
						value = boolArr.ToArray ();
					}
				}
				else if (t == typeof(double[]))
				{
					List<LuaValue> arr = luaValue.toArray();
					if (arr != null)
					{
						List<double> doubleArr = new List<double> ();
						foreach (LuaValue item in arr) 
						{
							doubleArr.Add (item.toNumber ());
						}
						value = doubleArr.ToArray ();
					}
				}
				else if (t == typeof(float[]))
				{
					List<LuaValue> arr = luaValue.toArray();
					if (arr != null) 
					{
						List<float> floatArr = new List<float> ();
						foreach (LuaValue item in arr) 
						{
							floatArr.Add ((float)item.toNumber ());
						}
						value = floatArr.ToArray ();
					}
				}
				else if (t == typeof(string[]))
				{
					List<LuaValue> arr = luaValue.toArray();
					if (arr != null) 
					{
						List<string> floatArr = new List<string> ();
						foreach (LuaValue item in arr) 
						{
							floatArr.Add (item.toString ());
						}
						value = floatArr.ToArray ();
					}
				}
				else
				{
					List<LuaValue> arr = luaValue.toArray();
					if (arr != null)
					{
						ArrayList objArr = new ArrayList ();
						foreach (LuaValue item in arr) 
						{
							objArr.Add (item.toObject ());
						}
						value = objArr.ToArray ();
					}
				}
			}
			else if (typeof(IDictionary).IsAssignableFrom(t)) 
			{
				//字典
				Dictionary<string, LuaValue> map = luaValue.toMap();
				if (map != null) 
				{
					if (t.IsGenericType)
					{
						//为泛型
						Type[] types = t.GetGenericArguments();
						Type valueType = types [1];

						IDictionary dict = Activator.CreateInstance(t) as IDictionary;
						foreach (KeyValuePair<string, LuaValue> kv in map)
						{
							dict.Add (kv.Key, getNativeValueForLuaValue(valueType, kv.Value));
						}

						value = dict;
					}
					else if (typeof(Hashtable).IsAssignableFrom(t))
					{
						Hashtable dict = new Hashtable ();
						foreach (KeyValuePair<string, LuaValue> kv in map)
						{
							dict.Add (kv.Key, kv.Value.toObject ());
						}
						value = dict;
					}
					
				}
			}
			else
			{
				value = luaValue.toObject();
			}

			return value;
		}

		/// <summary>
		/// 从原生编码对象中获取参数列表
		/// </summary>
		/// <returns>参数列表.</returns>
		/// <param name="context">上下文</param>
		/// <param name="arguments">参数缓存.</param>
		/// <param name="size">缓存长度.</param>
		protected static List<LuaValue> getArgumentList(LuaContext context, IntPtr arguments, int size)
		{
			List<LuaValue> argumentsList = null;
			if (arguments != IntPtr.Zero) 
			{
				//反序列化参数列表
				LuaObjectDecoder decoder = new LuaObjectDecoder(arguments, size, context);
				int argSize = decoder.readInt32 ();

				argumentsList = new List<LuaValue> ();
				for (int i = 0; i < argSize; i++) {

					LuaValue value = decoder.readObject () as LuaValue;
					argumentsList.Add (value);
				}
			}

			return argumentsList;
		}

		/// <summary>
		/// 解析方法的参数列表
		/// </summary>
		/// <returns>参数列表</returns>
		/// <param name="context">上下文对象</param>
		/// <param name="m">方法信息</param>
		/// <param name="arguments">参数列表数据</param>
		/// <param name="size">参数列表数据长度</param>
		protected static ArrayList parseMethodParameters(MethodInfo m, List<LuaValue> arguments)
		{
			ArrayList argsArr = null;
			ParameterInfo[] parameters = m.GetParameters ();
			if (parameters.Length > 0) 
			{
				int i = 0;
				argsArr = new ArrayList ();
				foreach (ParameterInfo p in parameters) 
				{
					if (i >= arguments.Count)
					{
						argsArr.Add (null);
					}
					else
					{
						object value = getNativeValueForLuaValue(p.ParameterType, arguments[i]);
						argsArr.Add (value);
					}

					i++;
				}
			}

			return argsArr;
		}
	}
}

