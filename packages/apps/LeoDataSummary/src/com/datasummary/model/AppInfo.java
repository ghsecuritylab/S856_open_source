package com.datasummary.model;

import android.Manifest.permission_group;
import android.R.string;
import android.content.pm.PackageInfo;
import android.content.pm.PermissionInfo;
import android.graphics.drawable.Drawable;

public class AppInfo {
	public String[] permissions;
	public String appName = "";
	public String packageName = "";
	public String versionName = "";
	public boolean bool= false;
	public Drawable appIcon = null;
	public int uid;
	public double rate=0.0;


	@Override
	public String toString() {
		// TODO Auto-generated method stub
		return super.toString();
	}
}
