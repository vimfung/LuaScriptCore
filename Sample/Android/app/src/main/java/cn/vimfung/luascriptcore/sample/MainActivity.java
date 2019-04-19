package cn.vimfung.luascriptcore.sample;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Build;
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
import java.sql.Time;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;

import cn.vimfung.luascriptcore.LuaContext;
import cn.vimfung.luascriptcore.LuaExceptionHandler;
import cn.vimfung.luascriptcore.LuaMethodHandler;
import cn.vimfung.luascriptcore.LuaScriptController;
import cn.vimfung.luascriptcore.LuaTuple;
import cn.vimfung.luascriptcore.LuaValue;


public class MainActivity extends AppCompatActivity {

    final Object obj = new Object();
    private LuaContext _luaContext;
    private LuaScriptController _scriptController;
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
        final Button evalScriptBtn = (Button) findViewById(R.id.evalScriptButton);
        if (evalScriptBtn != null)
        {
            evalScriptBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {

                    LuaValue retValue = _luaContext.evalScript("print(10);return 'Hello World';");
                    Log.v("lsc", retValue.toString());

                }
            });
        }

        //注册方法按钮点击
        final Button regMethodBtn = (Button) findViewById(R.id.regMethodButton);
        if (regMethodBtn != null)
        {
            regMethodBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {

//                    _scriptController.forceExit();

                    if (!_hasRegMethod)
                    {
                        //注册方法
                        _luaContext.registerMethod("getDeviceInfo", new LuaMethodHandler() {
                            @SuppressLint("SetTextI18n")
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
                    _luaContext.evalScript("LogModule:writeLog('Hello Lua Module!');");
                    _luaContext.evalScript("local obj = LogModule:createObj(); print(obj); LogModule:testObj(obj);");
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
//                    _luaContext.evalScript("local p = Person:createPersonError(); print(p);");
                    _luaContext.evalScript("print(Chinese); function Chinese.prototype:init() print('Chinese create'); end; local person = Chinese(); print(person); person.name = 'vimfung'; print(person.name); person:speak(); person:walk();");
                    _luaContext.evalScript("print(Person); local obj = Person:createObj(); Person:CheckObj(obj);");
                    _luaContext.evalScript("local p = Chinese(\"vimfung\"); print(p.name);");
                    _luaContext.evalScript("local p = Person(); print(\"int value\", p:getIntArray()[1][1]); print(\"Integer value\", p:getIntegerArray()[1][1]);" +
                            " print(\"long value\", p:getLongArray()[1][1]); print(\"Long value\", p:getLong2Array()[1][1]); " +
                            "print(\"float value\", p:getFloatArray()[1][1]); print(\"Float value\", p:getFloat2Array()[1][1]); " +
                            "print(\"double value\", p:getDoubleArray()[1][1]); print(\"Double value\", p:getDouble2Array()[1][1]);" +
                            " print(\"boolean value\", p:getBooleanArray()[1][1]); print(\"Boolean value\", p:getBoolean2Array()[1][1]);");
                    _luaContext.evalScript("local p = Person(); print(p:callFloatMethod(20)); print(p:callFloat2Method(20));");
                    _luaContext.evalScript("local p = Person(); for i=1, 1000 do p:log(\"Hello World\"); end");

                }
            });
        }

        Button importNativeClssBtn = (Button) findViewById(R.id.classProxyButton);
        if (importNativeClssBtn != null)
        {
            importNativeClssBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    _luaContext.evalScript("print(Person); local Data = NativeData; print(Data); local d = Data(); d:setData('key', 'xxx'); print(d:getData('key')); local p = Data:createPerson(); print(p); p.name = 'vim'; print(p.name); p:speak(); Person:printPersonName(p);");
                }
            });
        }

        Button retainReleaseBtn = (Button) findViewById(R.id.retainReleaseButton);
        if (retainReleaseBtn != null)
        {
            retainReleaseBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    _luaContext.evalScript("local test = function() print('test func') end; test(); Person:retainHandler2(test);");
                    _luaContext.evalScript("print('-------------1'); Person:callHandler2(); Person:releaseHandler2();");
                    _luaContext.evalScript("print('-------------2'); Person:callHandler2();");
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

        Button modulesBtn = (Button) findViewById(R.id.modulesButton);
        if (modulesBtn != null)
        {
            modulesBtn.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {

                    Intent intent = new Intent(MainActivity.this, ModulesActivity.class);
                    startActivity(intent);

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
