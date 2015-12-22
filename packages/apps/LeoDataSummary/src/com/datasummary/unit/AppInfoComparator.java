package com.datasummary.unit;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

import com.datasummary.model.AppInfo;

public class AppInfoComparator implements Comparator<AppInfo> {

	@Override
	public int compare(AppInfo app1, AppInfo app2) {
		// TODO Auto-generated method stub
		double rate1=app1.rate;
		double rate2=app2.rate;
		if(rate1>rate2){
			return -1;
		}else if(rate1<rate2){
			return 1;
		}else {
			return 0;
		}
		
	}
	public static ArrayList<AppInfo> Appinfossort(ArrayList<AppInfo> arrayList){
		
		AppInfoComparator wt = new AppInfoComparator();    
	    Collections.sort(arrayList, wt);    
	    return arrayList;
	}

}
