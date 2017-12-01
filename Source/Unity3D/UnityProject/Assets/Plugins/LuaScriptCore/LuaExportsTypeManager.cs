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
		/// 默认管理器对象
		/// </summary>
		static private LuaExportsTypeManager _manager = new LuaExportsTypeManager();

		/// <summary>
		/// 获取默认lua导出管理器
		/// </summary>
		/// <value>管理器.</value>
		static internal LuaExportsTypeManager defaultManager
		{
			get
			{
				return _manager;
			}
		}

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
		/// 导出类型标识映射表
		/// </summary>
		private static Dictionary<Type, int> _exportsClassIdMapping = new Dictionary<Type, int>();

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
		/// 导出类型
		/// </summary>
		/// <param name="type">类型.</param>
		/// <param name="context">上下文对象.</param>
		public void exportType(Type t, LuaContext context)
		{
			LuaExportTypeAnnotation typeAnnotation = Attribute.GetCustomAttribute (t, typeof(LuaExportTypeAnnotation), false) as LuaExportTypeAnnotation;
			LuaExportTypeAnnotation baseTypeAnnotation = Attribute.GetCustomAttribute (t.BaseType, typeof(LuaExportTypeAnnotation), false) as LuaExportTypeAnnotation;

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
			Dictionary<string, PropertyInfo> exportFields = new Dictionary<string, PropertyInfo> ();
			List<string> exportPropertyNames = new List<string> ();

			PropertyInfo[] propertys = t.GetProperties (BindingFlags.Instance | BindingFlags.Public);
			foreach (PropertyInfo p in propertys) 
			{
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
				exportFields.Add (p.Name, p);
			}

			//创建导出的字段数据
			IntPtr exportPropertyNamesPtr = IntPtr.Zero;
			if (exportPropertyNames.Count > 0)
			{
				LuaObjectEncoder fieldEncoder = new LuaObjectEncoder ();
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

			string typeName = t.Name;
			if (typeAnnotation != null && typeAnnotation.typeName != null)
			{
				typeName = typeAnnotation.typeName;
			}

			string baseTypeName = t.BaseType.Name;
			if (baseTypeAnnotation != null && baseTypeAnnotation.typeName != null)
			{
				baseTypeName = baseTypeAnnotation.typeName;
			}
				
			int typeId = NativeUtils.registerType (
				context.objectId, 
				typeName,
				baseTypeName,
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
			_exportsClassIdMapping [t] = typeId;
			_exportsClassMethods[typeId] = exportClassMethods;
			_exportsInstanceMethods[typeId] = exportInstanceMethods;
			_exportsFields[typeId] = exportFields;

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
		/// 根据C#类型获取原生类型标识
		/// </summary>
		/// <returns>原生类型标识.</returns>
		/// <param name="type">C#类型.</param>
		internal int getNativeTypeId(Type type)
		{
			if (_exportsClassIdMapping.ContainsKey (type))
			{
				return _exportsClassIdMapping [type];
			}

			return 0;
		}

		/// <summary>
		/// 创建实例对象
		/// </summary>
		/// <param name="nativeClassId">原生类型标识.</param>
		/// <param name="instanceId">实例标识</param>
		[MonoPInvokeCallback (typeof (LuaInstanceCreateHandleDelegate))]
		private static Int64 _createInstance(int nativeClassId)
		{
			Int64 refId = 0;
			Type t = _exportsClass [nativeClassId];
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
						_instances.Add(objRef);

						refId = objRef.referenceId;
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
		private static IntPtr _fieldGetter (int classId, Int64 instancePtr, string fieldName)
		{
			IntPtr retValuePtr = IntPtr.Zero;
			if (instancePtr != 0 
				&& _exportsFields.ContainsKey(classId) 
				&& _exportsFields[classId].ContainsKey(fieldName))
			{
				LuaObjectReference objRef = LuaObjectReference.findObject (instancePtr);
				object instance = objRef.target;
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
		private static void _fieldSetter (int classId, Int64 instancePtr, string fieldName, IntPtr valueBuffer, int bufferSize)
		{
			if (instancePtr != 0
				&& _exportsFields.ContainsKey(classId) 
				&& _exportsFields[classId].ContainsKey(fieldName))
			{
				LuaObjectReference objRef = LuaObjectReference.findObject (instancePtr);
				object instance = objRef.target;
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
		private static IntPtr _instanceMethodHandler (int classId, Int64 instancePtr, string methodName, IntPtr argumentsBuffer, int bufferSize)
		{
			if (instancePtr != 0
				&& _exportsInstanceMethods.ContainsKey(classId)
				&& _exportsInstanceMethods[classId].ContainsKey(methodName))
			{
				LuaObjectReference objRef = LuaObjectReference.findObject (instancePtr);
				object instance = objRef.target;
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
			else if (t.IsArray) 
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
			else if (t == typeof(Hashtable)) 
			{
				//字典
				Dictionary<string, LuaValue> map = luaValue.toMap();
				if (map != null) 
				{
					if (t == typeof(Dictionary<string, int>))
					{
						Dictionary<string, int> dict = new Dictionary<string, int> ();
						foreach (KeyValuePair<string, LuaValue> kv in map) 
						{
							dict.Add (kv.Key, kv.Value.toInteger ());
						}
						value = dict;
					} 
					else if (t == typeof(Dictionary<string, int>))
					{
						Dictionary<string, int> dict = new Dictionary<string, int> ();
						foreach (KeyValuePair<string, LuaValue> kv in map) 
						{
							dict.Add (kv.Key, kv.Value.toInteger ());
						}
						value = dict;
					} 
					else if (t == typeof(Dictionary<string, uint>))
					{
						Dictionary<string, uint> dict = new Dictionary<string, uint> ();
						foreach (KeyValuePair<string, LuaValue> kv in map)
						{
							dict.Add (kv.Key, Convert.ToUInt32 (kv.Value.toInteger ()));
						}
						value = dict;
					}
					else if (t == typeof(Dictionary<string, Int16>))
					{
						Dictionary<string, Int16> dict = new Dictionary<string, Int16> ();
						foreach (KeyValuePair<string, LuaValue> kv in map) 
						{
							dict.Add (kv.Key, Convert.ToInt16 (kv.Value.toInteger ()));
						}
						value = dict;
					}
					else if (t == typeof(Dictionary<string, UInt16>))
					{
						Dictionary<string, UInt16> dict = new Dictionary<string, UInt16> ();
						foreach (KeyValuePair<string, LuaValue> kv in map) 
						{
							dict.Add (kv.Key, Convert.ToUInt16 (kv.Value.toInteger ()));
						}
						value = dict;
					}
					else if (t == typeof(Dictionary<string, Int64>))
					{
						Dictionary<string, Int64> dict = new Dictionary<string, Int64> ();
						foreach (KeyValuePair<string, LuaValue> kv in map)
						{
							dict.Add (kv.Key, Convert.ToInt64 (kv.Value.toInteger ()));
						}
						value = dict;
					}
					else if (t == typeof(Dictionary<string, UInt64>))
					{
						Dictionary<string, UInt64> dict = new Dictionary<string, UInt64> ();
						foreach (KeyValuePair<string, LuaValue> kv in map)
						{
							dict.Add (kv.Key, Convert.ToUInt64 (kv.Value.toInteger ()));
						}
						value = dict;
					} 
					else if (t == typeof(Dictionary<string, bool>))
					{
						Dictionary<string, bool> dict = new Dictionary<string, bool> ();
						foreach (KeyValuePair<string, LuaValue> kv in map)
						{
							dict.Add (kv.Key, kv.Value.toBoolean ());
						}
						value = dict;
					} 
					else if (t == typeof(Dictionary<string, double>)) 
					{
						Dictionary<string, double> dict = new Dictionary<string, double> ();
						foreach (KeyValuePair<string, LuaValue> kv in map)
						{
							dict.Add (kv.Key, kv.Value.toNumber ());
						}
						value = dict;
					}
					else if (t == typeof(Dictionary<string, float>)) 
					{
						Dictionary<string, float> dict = new Dictionary<string, float> ();
						foreach (KeyValuePair<string, LuaValue> kv in map)
						{
							dict.Add (kv.Key, (float)kv.Value.toNumber ());
						}
						value = dict;
					}
					else if (t == typeof(Dictionary<string, string>))
					{
						Dictionary<string, string> dict = new Dictionary<string, string> ();
						foreach (KeyValuePair<string, LuaValue> kv in map) 
						{
							dict.Add (kv.Key, kv.Value.toString ());
						}
						value = dict;
					}
					else 
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
		/// 解析方法的参数列表
		/// </summary>
		/// <returns>参数列表</returns>
		/// <param name="m">方法信息</param>
		/// <param name="arguments">参数列表数据</param>
		/// <param name="size">参数列表数据长度</param>
		protected static ArrayList parseMethodParameters(MethodInfo m, IntPtr arguments, int size)
		{
			List<LuaValue> argumentsList = null;
			if (arguments != IntPtr.Zero) 
			{
				//反序列化参数列表
				LuaObjectDecoder decoder = new LuaObjectDecoder(arguments, size);
				int argSize = decoder.readInt32 ();

				argumentsList = new List<LuaValue> ();
				for (int i = 0; i < argSize; i++) {

					LuaValue value = decoder.readObject () as LuaValue;
					argumentsList.Add (value);
				}
			}

			ArrayList argsArr = null;
			ParameterInfo[] parameters = m.GetParameters ();
			if (parameters.Length > 0 && argumentsList != null) 
			{
				int i = 0;
				argsArr = new ArrayList ();
				foreach (ParameterInfo p in parameters) 
				{
					if (i >= argumentsList.Count) 
					{
						break;
					}

					object value = getNativeValueForLuaValue(p.ParameterType, argumentsList[i]);
					argsArr.Add (value);

					i++;
				}
			}

			return argsArr;
		}
	}
}

