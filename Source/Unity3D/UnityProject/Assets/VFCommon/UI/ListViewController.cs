using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using EnhancedUI.EnhancedScroller;
using System;

namespace cn.vimfung.game.u3d.ui
{
	/// <summary>
	/// 列表视图控制器
	/// </summary>
	public class ListViewController : MonoBehaviour, IEnhancedScrollerDelegate
	{
		/// <summary>
		/// 数据源
		/// </summary>
		private IList _dataSource;

		/// <summary>
		/// 数据源
		/// </summary>
		public IList dataSource
		{
			get
			{
				return _dataSource;
			}
			set
			{
				_dataSource = value;
				scroller.ReloadData ();
			}
		}

		/// <summary>
		/// 滚动栏
		/// </summary>
		public EnhancedScroller scroller;

		/// <summary>
		/// 单元格预设
		/// </summary>
		public EnhancedScrollerCellView cellPrefab;

		/// <summary>
		/// 行高
		/// </summary>
		public float rowHeight = 120;

		/// <summary>
		/// 列表项渲染委托
		/// </summary>
		public Action<EnhancedScrollerCellView, object> itemRendererHandler;

		/// <summary>
		/// 重新加载数据
		/// </summary>
		public void reloadData ()
		{
			scroller.ReloadData ();
		}

		/// <summary>
		/// Gets the number of cells in a list of data
		/// </summary>
		/// <param name="scroller"></param>
		/// <returns></returns>
		public int GetNumberOfCells(EnhancedScroller scroller)
		{
			if (dataSource != null)
			{
				return dataSource.Count;
			}

			return 0;
		}

		/// <summary>
		/// Gets the size of a cell view given the index of the data set.
		/// This allows you to have different sized cells
		/// </summary>
		/// <param name="scroller"></param>
		/// <param name="dataIndex"></param>
		/// <returns></returns>
		public float GetCellViewSize(EnhancedScroller scroller, int dataIndex)
		{
			return rowHeight;
		}

		/// <summary>
		/// Gets the cell view that should be used for the data index. Your implementation
		/// of this function should request a new cell from the scroller so that it can
		/// properly recycle old cells.
		/// </summary>
		/// <param name="scroller"></param>
		/// <param name="dataIndex"></param>
		/// <param name="cellIndex"></param>
		/// <returns></returns>
		public EnhancedScrollerCellView GetCellView(EnhancedScroller scroller, int dataIndex, int cellIndex)
		{
			EnhancedScrollerCellView cellView = scroller.GetCellView (cellPrefab);
			cellView.dataIndex = dataIndex;

			if (itemRendererHandler != null)
			{
				itemRendererHandler (cellView, dataSource != null && dataSource.Count > dataIndex ? dataSource [dataIndex] : null);
			}

			return cellView;
		}

		void Start ()
		{
			scroller.Delegate = this;
		}
	}

}
