package cn.vimfung.luascriptcore.sample;

import android.app.Application;
import android.bluetooth.BluetoothClass;
import android.os.Build;
import android.os.Debug;
import android.provider.Settings;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.HashMap;

import cn.vimfung.luascriptcore.LuaContext;
import cn.vimfung.luascriptcore.LuaExceptionHandler;
import cn.vimfung.luascriptcore.LuaFunction;
import cn.vimfung.luascriptcore.LuaMethodHandler;
import cn.vimfung.luascriptcore.LuaTuple;
import cn.vimfung.luascriptcore.LuaValue;


public class MainActivity extends AppCompatActivity {

    private LuaContext _luaContext;
    private boolean _hasRegMethod;
    private boolean _hasRegModule;
    private boolean _hasRegClass;
    private boolean _useClassProxy;
    private boolean _hasRegCoroutineMethod;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //创建LuaContext
        Env.setup(this);
        _luaContext = Env.defaultContext();

        _luaContext.onException(new LuaExceptionHandler() {
            @Override
            public void onException(String message) {

                Log.v("lua exception log", message);

            }
        });

        //解析脚本按钮点击
        Button evalScriptBtn = (Button) findViewById(R.id.evalScriptButton);
        if (evalScriptBtn != null)
        {
            evalScriptBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    LuaValue retValue = _luaContext.evalScript("print(10); return 'Hello','World';");
                    Log.v("luaScriptCoreSample", retValue.toString());
                }
            });
        }

        //注册方法按钮点击
        Button regMethodBtn = (Button) findViewById(R.id.regMethodButton);
        if (regMethodBtn != null)
        {
            regMethodBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {

                    if (!_hasRegMethod)
                    {
                        //注册方法
                        _luaContext.registerMethod("getDeviceInfo", new LuaMethodHandler() {
                            @Override
                            public LuaValue onExecute(LuaValue[] arguments) {

                                HashMap devInfoMap = new HashMap();
                                devInfoMap.put("deviceName", Build.DISPLAY);
                                devInfoMap.put("deviceModel", Build.MODEL);
                                devInfoMap.put("systemName", Build.PRODUCT);
                                devInfoMap.put("systemVersion", Build.VERSION.RELEASE);

                                return new LuaValue(devInfoMap);

                            }
                        });

                        _hasRegMethod = true;
                    }

                    //调用脚本
                    _luaContext.evalScriptFromFile("main.lua");

                }
            });
        }


        //调用方法按钮
        Button callMethodBtn = (Button) findViewById(R.id.callLuaMethodButton);
        if (callMethodBtn != null)
        {
            callMethodBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {

                    Log.v("luaScriptCore", "callMethodBtn clicked.");

                    //调用脚本
                    File todoFile = new File ("todo.lua");
                    _luaContext.evalScriptFromFile(todoFile.toString());

                    LuaValue retValue = _luaContext.callMethod("add", new LuaValue[]{new LuaValue(100), new LuaValue(924)});
                    Log.v("luaScriptCore",String.format("%d", retValue.toInteger()));

                    LuaValue funcValue = _luaContext.callMethod("getFunc", null);
                    retValue = funcValue.toFunction().invoke(new LuaValue[]{new LuaValue(100), new LuaValue(924)});
                    Log.v("luaScriptCore",String.format("%d", retValue.toInteger()));

                }
            });
        }

        //注册模块按钮
        Button regModuleBtn = (Button) findViewById(R.id.regModuleButton);
        if (regModuleBtn != null)
        {
            regModuleBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    _luaContext.evalScript("LogModule.writeLog('Hello Lua Module!');");
                    _luaContext.evalScript("local obj = LogModule.createObj(); print(obj); LogModule.testObj(obj);");
                }
            });
        }

        //注册类按钮
        Button regClsBtn = (Button) findViewById(R.id.regClassButton);
        if (regClsBtn != null)
        {
            regClsBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
//                    _luaContext.evalScript("print(Chinese); function Chinese.prototype:init() print('Chinese create'); end; local person = Chinese.create(); print(person); person.name = 'vimfung'; print(person.name); person:speak(); person:walk();");
                    _luaContext.evalScript("local obj = Person.createObj(); Person.CheckObj(obj);");
                }
            });
        }

        Button importNativeClssBtn = (Button) findViewById(R.id.classProxyButton);
        if (importNativeClssBtn != null)
        {
            importNativeClssBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    _luaContext.evalScript("print(Person); local Data = NativePerson; print(Data); local d = Data.create(); d:setData('key', 'xxx'); print(d:getData('key')); local p = Data.createPerson(); print(p); p.name = 'vim'; print(p.name); p:speak(); Person.printPersonName(p);");
                }
            });
        }

        Button retainReleaseBtn = (Button) findViewById(R.id.retainReleaseButton);
        if (retainReleaseBtn != null)
        {
            retainReleaseBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    _luaContext.evalScript("local test = function() print('test func') end; test(); Person.retainHandler2(test);");
                    _luaContext.evalScript("print('-------------1'); Person.callHandler2(); Person.releaseHandler2();");
                    _luaContext.evalScript("print('-------------2'); Person.callHandler2();");
                }
            });
        }

        Button coroutineBtn = (Button) findViewById(R.id.coroutineButton);
        if (coroutineBtn != null)
        {
            coroutineBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {

                    if (!_hasRegCoroutineMethod) {

                        _hasRegCoroutineMethod = true;
                        _luaContext.registerMethod("GetValue", new LuaMethodHandler() {
                            @Override
                            public LuaValue onExecute(LuaValue[] arguments) {

                                return new LuaValue(1024);
                            }
                        });

                        _luaContext.registerMethod("GetPixel", new LuaMethodHandler() {
                            @Override
                            public LuaValue onExecute(LuaValue[] arguments) {

                                LuaTuple tuple = new LuaTuple();
                                tuple.addReturnValue(100);
                                tuple.addReturnValue(20);
                                tuple.addReturnValue(232);

                                return new LuaValue(tuple);
                            }
                        });
                    }

                    _luaContext.evalScriptFromFile("coroutine.lua");

                }
            });
        }

        Button definedPropertyBtn = (Button) findViewById(R.id.definedPropertyButton);
        if (definedPropertyBtn != null)
        {
            definedPropertyBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {

                    _luaContext.evalScriptFromFile("defineProperty.lua");

                }
            });
        }
    }

    /**
     * 读取资源文件内容
     * @param fileName  文件名称
     * @param outputStream  输出内容的二进制流
     */
    private void readAssetFileContent(String fileName, ByteArrayOutputStream outputStream)
    {
        try
        {
            InputStream stream = getAssets().open(fileName);
            byte[] buffer = new byte[1024];

            int hasRead = 0;
            while ((hasRead = stream.read(buffer)) != -1)
            {
                outputStream.write(buffer, 0, hasRead);
            }

            stream.close();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

    /**
     * 写入文件
     * @param file  目标文件
     * @param dataStream    二进制流
     */
    private void writeToFile(File file, ByteArrayOutputStream dataStream)
    {
        try
        {
            FileOutputStream stream = new FileOutputStream(file);
            dataStream.writeTo(stream);
            stream.close();
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }
}
