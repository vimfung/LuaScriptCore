using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using cn.vimfung.game.u3d.ui;
using System;
using cn.vimfung.luascriptcore;

public class ModulesSceneController : MonoBehaviour {

	public ListViewController listVC;

	private string _selectedItem = null;

	public void backButtonClickedHandler()
	{
		if (_selectedItem == null)
		{
			SceneManager.LoadScene ("Scene1");
		}
		else
		{
			Debug.LogFormat ("_selectedItem = {0}", _selectedItem);
			switch (_selectedItem)
			{
			case "Foundation":
			case "Network":
				_selectedItem = null;
				listVC.dataSource = new List<string> () {"Foundation", "Network"};
				listVC.reloadData ();
				break;
			case "Encoding":
			case "Path":
			case "Thread":
				_selectedItem = "Foundation";
				listVC.dataSource = new List<string> () {"Encoding", "Path", "Thread"};
				listVC.reloadData ();
				break;
			case "HTTP":
				_selectedItem = "Network";
				listVC.dataSource = new List<string> () {"HTTP"};
				listVC.reloadData ();
				break;
			}
		}
	}

	void Start()
	{
		LuaContext context = LuaContext.currentContext;

		context.addSearchPath ("LuaScriptCore_Modules/Foundation");
		context.addSearchPath ("LuaScriptCore_Modules/Network");

		context.onException ((msg) => {

			Debug.LogFormat("lua exception = {0}", msg);

		});

		listVC.itemRendererHandler = (cell, data) => {

			ModuleItemCell itemCell = cell.GetComponent<ModuleItemCell>();

			string itemData = Convert.ToString(data);
			itemCell.titleLabel.text = itemData;

			itemCell.itemClickHandler = () => {

				switch (itemData)
				{
				case "Foundation":
					_selectedItem = itemData;
					listVC.dataSource = new List<string> () {"Encoding", "Path", "Thread"};
					listVC.reloadData();
					break;
				case "Network":
					_selectedItem = itemData;
					listVC.dataSource = new List<string> () {"HTTP"};
					listVC.reloadData();
					break;
				case "Encoding":
					_selectedItem = itemData;
					listVC.dataSource = new List<string> () {"URL Encode", "URL Decode", "Base64 Encode", "Base64 Decode", "JSON Encode", "JSON Decode"};
					listVC.reloadData();
					break;
				case "Path":
					_selectedItem = itemData;
					listVC.dataSource = new List<string> () {"App Path", "Home Path", "Documents Path", "Caches Path", "Tmp Path", "Exists Path"};
					listVC.reloadData();
					break;
				case "Thread":
					_selectedItem = itemData;
					listVC.dataSource = new List<string> () {"Start Thread", "Stop Thread"};
					listVC.reloadData();
					break;
				case "HTTP":
					_selectedItem = itemData;
					listVC.dataSource = new List<string> () {"GET Request", "POST Request", "Upload File", "Download File"};
					listVC.reloadData();
					break;
				case "URL Encode":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Foundation/Sample/Encoding-Sample.lua");
					LuaContext.currentContext.evalScript("Encoding_Sample_urlEncode()");
					break;
				case "URL Decode":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Foundation/Sample/Encoding-Sample.lua");
					LuaContext.currentContext.evalScript("Encoding_Sample_urlDecode()");
					break;
				case "Base64 Encode":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Foundation/Sample/Encoding-Sample.lua");
					LuaContext.currentContext.evalScript("Encoding_Sample_base64Encode()");
					break;
				case "Base64 Decode":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Foundation/Sample/Encoding-Sample.lua");
					LuaContext.currentContext.evalScript("Encoding_Sample_base64Decode()");
					break;
				case "JSON Encode":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Foundation/Sample/Encoding-Sample.lua");
					LuaContext.currentContext.evalScript("Encoding_Sample_jsonEndode()");
					break;
				case "JSON Decode":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Foundation/Sample/Encoding-Sample.lua");
					LuaContext.currentContext.evalScript("Encoding_Sample_jsonDecode()");
					break;
				case "App Path":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Foundation/Sample/Path-Sample.lua");
					LuaContext.currentContext.evalScript("Path_Sample_appPath()");
					break;
				case "Home Path":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Foundation/Sample/Path-Sample.lua");
					LuaContext.currentContext.evalScript("Path_Sample_homePath()");
					break;
				case "Documents Path":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Foundation/Sample/Path-Sample.lua");
					LuaContext.currentContext.evalScript("Path_Sample_docsPath()");
					break;
				case "Caches Path":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Foundation/Sample/Path-Sample.lua");
					LuaContext.currentContext.evalScript("Path_Sample_cachesPath()");
					break;
				case "Tmp Path":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Foundation/Sample/Path-Sample.lua");
					LuaContext.currentContext.evalScript("Path_Sample_tmpPath()");
					break;
				case "Exists Path":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Foundation/Sample/Path-Sample.lua");
					LuaContext.currentContext.evalScript("Path_Sample_exists()");
					break;
				case "GET Request":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Network/Sample/HTTP-Sample.lua");
					LuaContext.currentContext.evalScript("HTTP_Sample_get()");
					break;
				case "POST Request":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Network/Sample/HTTP-Sample.lua");
					LuaContext.currentContext.evalScript("HTTP_Sample_post()");
					break;
				case "Upload File":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Network/Sample/HTTP-Sample.lua");
					LuaContext.currentContext.evalScript("HTTP_Sample_upload()");
					break;
				case "Download File":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Network/Sample/HTTP-Sample.lua");
					LuaContext.currentContext.evalScript("HTTP_Sample_download()");
					break;
				case "Start Thread":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Foundation/Sample/Thread-Sample.lua");
					LuaContext.currentContext.evalScript("Thread_Sample_run()");
					break;
				case "Stop Thread":
					LuaContext.currentContext.evalScriptFromFile("LuaScriptCore_Modules/Foundation/Sample/Thread-Sample.lua");
					LuaContext.currentContext.evalScript("Thread_Sample_stop()");
					break;
				}

			};

		};

		listVC.dataSource = new List<string> () {"Foundation", "Network"};
		listVC.reloadData ();
	}

}
