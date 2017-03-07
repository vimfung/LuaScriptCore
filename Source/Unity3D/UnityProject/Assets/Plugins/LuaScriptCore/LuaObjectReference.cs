using System;
using System.Collections.Generic;

namespace cn.vimfung.luascriptcore
{
	internal class LuaObjectReference : object
	{
		static int _referenceIdSeed = 0;
		static private Dictionary<Int64, LuaObjectReference> _references = new Dictionary<Int64, LuaObjectReference>();
		private Int64 _objectRefId = 0;
		private object _target = null;

		public LuaObjectReference (object target)
		{
			_target = target;
			createReferenceIndex ();
		}

		~LuaObjectReference()
		{
			//移除引用对象
			_references.Remove (_objectRefId);
		}

		/// <summary>
		/// 创建引用索引
		/// </summary>
		private void createReferenceIndex()
		{
			_referenceIdSeed++;
			_objectRefId = _referenceIdSeed;
			_references [_objectRefId] = this;
		}

		/// <summary>
		/// 获取引用标识
		/// </summary>
		/// <value>引用标识.</value>
		public Int64 referenceId
		{
			get
			{
				return _objectRefId;
			}
		}

		/// <summary>
		/// 获取目标对象
		/// </summary>
		/// <value>目标对象.</value>
		public object target
		{
			get
			{
				return _target;
			}
		}

		/// <summary>
		/// 查找对象
		/// </summary>
		/// <returns>对象.</returns>
		/// <param name="referenceId">引用标识.</param>
		public static LuaObjectReference findObject(Int64 referenceId)
		{
			if (_references.ContainsKey(referenceId))
			{
				return _references [referenceId];
			}

			return null;
		}
	}
}

