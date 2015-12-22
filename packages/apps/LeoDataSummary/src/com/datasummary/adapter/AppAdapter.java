package com.datasummary.adapter;

import java.util.List;

import com.datasummary.LeoDataSummary.MainActivity;
import com.datasummary.LeoDataSummary.R;
import com.datasummary.LeoDataSummary.R.id;
import com.datasummary.model.AppInfo;
import com.datasummary.unit.LeoDataSummaryLog;
import java.text.DecimalFormat;
import android.R.integer;
import android.content.Context;
import android.os.Bundle;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.ToggleButton;
import android.widget.Switch;

import android.widget.CompoundButton.OnCheckedChangeListener;
import android.os.SystemProperties;

/*
 * listadapter
 */
public class AppAdapter extends BaseAdapter {
	public static String TAG="AppAdapter";
	Context mcontext;
	List<AppInfo> mlist;
	int mitemlayoutid;
	LayoutInflater mLayoutInflater;
	public double maxrate=1.0;

	public AppAdapter(Context context, List<AppInfo> appinfodata,
			int itemlayoutid) {
		mcontext = context;
		mitemlayoutid = itemlayoutid;
		mlist = appinfodata;
		mLayoutInflater = LayoutInflater.from(mcontext);
		for (int i = 0; i < mlist.size(); i++) {
			if(mlist.get(i).rate>maxrate){
				maxrate=mlist.get(i).rate;
			}
		}
	}

	@Override
	public int getCount() {

		// TODO Auto-generated method stub
		return mlist.size();
	}

	@Override
	public Object getItem(int position) {
		// TODO Auto-generated method stub
		return mlist.get(position);
	}

	@Override
	public long getItemId(int position) {
		// TODO Auto-generated method stub
		return mlist.hashCode();
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		// TODO Auto-generated method stub
		if (null == convertView) {
			convertView = mLayoutInflater.inflate(mitemlayoutid, null);

		}

		TextView nameTv = (TextView) convertView
				.findViewById(R.id.app_item_name);
		ImageView iconIv = (ImageView) convertView
				.findViewById(R.id.app_item_image);

		TextView versionTv = (TextView) convertView
				.findViewById(R.id.app_item_size);

		Switch toggleButtonTvButton = (Switch) convertView
				.findViewById(R.id.app_item_togglebutton);

		ProgressBar progressBarTvProgressBar=(ProgressBar) convertView.findViewById(R.id.app_item_progressbar);
		TextView rateTv=(TextView)convertView.findViewById(R.id.app_item_rateinfo);
		
		
		final int tmpposition=position;
		final AppInfo nowapp = mlist.get(position);
		iconIv.setImageDrawable(nowapp.appIcon);
		versionTv.setText(nowapp.uid+" ");
                /*lenovo-sw renjun1 begin modify for SHELLROW-1604 2014.6.30*/
                toggleButtonTvButton.setOnCheckedChangeListener(null);
		toggleButtonTvButton.setChecked(nowapp.bool);
		toggleButtonTvButton
		.setOnCheckedChangeListener(new OnCheckedChangeListener() {

			@Override
			public void onCheckedChanged(CompoundButton button, boolean isChecked) {
				// TODO Auto-generated method stub
                /*lenovo-sw renjun1 end modify for SHELLROW-1604 2014.6.30*/
				Message message=new Message();
				Bundle bundle=new Bundle();
				bundle.putString("packagename", nowapp.packageName);
				bundle.putInt("position", tmpposition);
				bundle.putInt("uid", nowapp.uid);
				message.setData(bundle);
				message.what=0x101;
				int uid = nowapp.uid;
				LeoDataSummaryLog.d(TAG,"position: "+tmpposition+" name: "+nowapp.packageName+" bool: "+nowapp.bool+"uid: "+uid);
				if(nowapp.bool) {
					SystemProperties.set("sys.jcommand_para","[DISABLEAPPNET]["+uid+"][null]");
					try {
						Thread.sleep(300);						
					}catch(Exception e) {

					}
					SystemProperties.set("ctl.start","jcommand");

				}else {
					SystemProperties.set("sys.jcommand_para","[ENABLEAPPNET]["+uid+"][null]");
					try {
						Thread.sleep(300);						
					}catch(Exception e) {

					}
					SystemProperties.set("ctl.start","jcommand");

				}
				MainActivity.handler.sendMessage(message);
				 

			}

		});

		if (nowapp.appName.equals(null)) {
			nameTv.setText("unkown");
		} else {
			nameTv.setText(nowapp.appName);
		}
		
			progressBarTvProgressBar.setProgress((int) (nowapp.rate/maxrate*100));

		DecimalFormat myformat = new DecimalFormat();
		myformat.applyPattern("####.00");
		/*
		fix SHELLW-1592 maozd1 20140429
		*/
		if(nowapp.rate==0.0){
			 rateTv.setText("  ");
        }else if(nowapp.rate<1024.0){
			rateTv.setText(nowapp.rate+"B");
		}else if(nowapp.rate<(1024*1024)){
			
			rateTv.setText(myformat.format(nowapp.rate/1024)+"KB");
		}else if(nowapp.rate<(1024*1024*1024)){
		rateTv.setText(myformat.format(nowapp.rate/1024/1024)+"MB");
			}else{
			rateTv.setText(myformat.format(nowapp.rate/1024/1024/1024)+"GB");
				}
		
		
		return convertView;
	}

}
