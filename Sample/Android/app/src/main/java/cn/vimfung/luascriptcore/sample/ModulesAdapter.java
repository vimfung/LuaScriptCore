package cn.vimfung.luascriptcore.sample;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import java.util.List;

public class ModulesAdapter extends ArrayAdapter<String>
{
    private int _resourceId;

    public ModulesAdapter(Context context,
                          int textViewResourceId,
                          List<String> objects)
    {
        super(context, textViewResourceId, objects);
        _resourceId = textViewResourceId;
    }

    @NonNull
    @Override
    public View getView(int position, @Nullable View convertView, @NonNull ViewGroup parent)
    {
        String item = getItem(position);
        View view;
        ViewHolder viewHolder;
        if (convertView == null)
        {
            //若没有缓存布局，则加载
            //首先获取布局填充器，然后使用布局填充器填充布局文件
            view = LayoutInflater.from(getContext()).inflate(_resourceId, null);
            viewHolder = new ViewHolder();
            //存储子项布局中子控件对象
            viewHolder.nameTextView = (TextView) view.findViewById(R.id.moduleNameTextView);
            // 将内部类对象存储到View对象中
            view.setTag(viewHolder);
        }
        else
        { //若有缓存布局，则直接用缓存（利用的是缓存的布局，利用的不是缓存布局中的数据）
            view = convertView;
            viewHolder = (ViewHolder) view.getTag();
        }

        viewHolder.nameTextView.setText(item);

        return view;
    }
}

class ViewHolder
{
    TextView nameTextView;
}