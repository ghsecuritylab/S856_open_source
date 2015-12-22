package com.datasummary.unit;


import com.datasummary.unit.LeoDataSummaryLog;


public class MyFile {
	public static String TAG="MyFile";
	public String filename="test.txt";
	public String s="com.android.phone 1";

	public String[] split_s(){
		String arrays[] = s.split(" ");
		LeoDataSummaryLog.d(TAG,"arr"+s);
		for (int i = 0; i < arrays.length; i++) {
			LeoDataSummaryLog.d(TAG,"arr"+arrays[i]);
		}
		return arrays;
	}
	

}
