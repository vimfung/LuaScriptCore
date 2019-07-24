package cn.vimfung.luascriptcore.sample;

import android.content.Intent;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import java.util.ArrayList;

import cn.vimfung.luascriptcore.LuaContext;

public class ModulesActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_modules);

        Bundle extras = getIntent().getExtras();
        String parentItem = null;
        if (extras != null)
        {
            parentItem = extras.getString("value");
        }


        ArrayList<String> modules = new ArrayList<>();
        if (parentItem == null)
        {
            modules.add("Encoding");
            modules.add("Path");
            modules.add("Thread");
            modules.add("HTTP");
            modules.add("Crypto");
        }
        else if (parentItem.equals("Encoding"))
        {
            modules.add("URL Encode");
            modules.add("URL Decode");
            modules.add("Base64 Encode");
            modules.add("Base64 Decode");
            modules.add("JSON Encode");
            modules.add("JSON Decode");
            modules.add("Hex Encode");
            modules.add("Hex Decode");
        }
        else if (parentItem.equals("Path"))
        {
            modules.add("App Path");
            modules.add("Home Path");
            modules.add("Documents Path");
            modules.add("Caches Path");
            modules.add("Tmp Path");
            modules.add("Exists Path");
        }
        else if (parentItem.equals("Thread"))
        {
            modules.add("Run Thread");
            modules.add("Stop Thread");
        }
        else if (parentItem.equals("HTTP"))
        {
            modules.add("GET Request");
            modules.add("POST Request");
            modules.add("Upload File");
            modules.add("Download File");
        }
        else if(parentItem.equals("Crypto"))
        {
            modules.add("MD5");
            modules.add("SHA1");
            modules.add("HMAC-MD5");
            modules.add("HMAC-SHA1");
        }

        final String finalParentItem = parentItem;
        final ModulesAdapter adapter = new ModulesAdapter(this, R.layout.module_list_item_view, modules);

        ListView listView = (ListView) findViewById(R.id.moduleListView);
        listView.setAdapter(adapter);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

                LuaContext luaContext = Env.defaultContext();
                String item = adapter.getItem(position);
                if (item != null)
                {
                    if (finalParentItem == null)
                    {
                        Intent intent = new Intent(ModulesActivity.this, ModulesActivity.class);
                        intent.putExtra("value", item);
                        startActivity(intent);
                    }
                    else if (finalParentItem.equals("Encoding"))
                    {
                        luaContext.evalScriptFromFile("Encoding-Sample.lua");

                        if (item.equals("URL Encode"))
                        {
                            luaContext.evalScript("Encoding_Sample_urlEncode()");
                        }
                        else if (item.equals("URL Decode"))
                        {
                            luaContext.evalScript("Encoding_Sample_urlDecode()");
                        }
                        else if (item.equals("Base64 Encode"))
                        {
                            luaContext.evalScript("Encoding_Sample_base64Encode()");
                        }
                        else if (item.equals("Base64 Decode"))
                        {
                            luaContext.evalScript("Encoding_Sample_base64Decode()");
                        }
                        else if (item.equals("JSON Encode"))
                        {
                            luaContext.evalScript("Encoding_Sample_jsonEndode()");
                        }
                        else if (item.equals("JSON Decode"))
                        {
                            luaContext.evalScript("Encoding_Sample_jsonDecode()");
                        }
                        else if (item.equals("Hex Encode"))
                        {
                            luaContext.evalScript("Encoding_Sample_hexEncode()");
                        }
                        else if (item.equals("Hex Decode"))
                        {
                            luaContext.evalScript("Encoding_Sample_hexDecode()");
                        }
                    }
                    else if (finalParentItem.equals("Path"))
                    {
                        luaContext.evalScriptFromFile("Path-Sample.lua");

                        if (item.equals("App Path"))
                        {
                            luaContext.evalScript("Path_Sample_appPath()");
                        }
                        else if (item.equals("Home Path"))
                        {
                            luaContext.evalScript("Path_Sample_homePath()");
                        }
                        else if (item.equals("Documents Path"))
                        {
                            luaContext.evalScript("Path_Sample_docsPath()");
                        }
                        else if (item.equals("Caches Path"))
                        {
                            luaContext.evalScript("Path_Sample_cachesPath()");
                        }
                        else if (item.equals("Tmp Path"))
                        {
                            luaContext.evalScript("Path_Sample_tmpPath()");
                        }
                        else  if (item.equals("Exists Path"))
                        {
                            luaContext.evalScript("Path_Sample_exists()");
                        }
                    }
                    else if (finalParentItem.equals("Thread"))
                    {
                        luaContext.evalScriptFromFile("Thread-Sample.lua");

                        if (item.equals("Run Thread"))
                        {
                            luaContext.evalScript("Thread_Sample_run()");
                        }
                        else if (item.equals("Stop Thread"))
                        {
                            luaContext.evalScript("Thread_Sample_stop()");
                        }
                    }
                    else if (finalParentItem.equals("HTTP"))
                    {
                        luaContext.evalScriptFromFile("HTTP-Sample.lua");

                        if (item.equals("GET Request"))
                        {
                            luaContext.evalScript("HTTP_Sample_get()");
                        }
                        else if (item.equals("POST Request"))
                        {
                            luaContext.evalScript("HTTP_Sample_post()");
                        }
                        else if (item.equals("Upload File"))
                        {
                            luaContext.evalScript("HTTP_Sample_upload()");
                        }
                        else if (item.equals("Download File"))
                        {
                            luaContext.evalScript("HTTP_Sample_download()");
                        }
                    }
                    else if(finalParentItem.equals("Crypto"))
                    {
                        luaContext.evalScriptFromFile("Crypto-Sample.lua");

                        if (item.equals("MD5"))
                        {
                            luaContext.evalScript("Crypto_Sample_md5()");
                        }
                        else if (item.equals("SHA1"))
                        {
                            luaContext.evalScript("Crypto_Sample_sha1()");
                        }
                        else if (item.equals("HMAC-MD5"))
                        {
                            luaContext.evalScript("Crypto_Sample_hmacMD5()");
                        }
                        else if (item.equals("HMAC-SHA1"))
                        {
                            luaContext.evalScript("Crypto_Sample_hmacSHA1()");
                        }
                    }
                }

            }
        });
    }

}
