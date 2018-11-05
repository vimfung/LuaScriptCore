using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System;

public class ModuleItemCell : MonoBehaviour
{
	public Text titleLabel;
	public Action itemClickHandler;

	public void clickHandler ()
	{
		if (itemClickHandler != null)
		{
			itemClickHandler ();
		}
	}
}
