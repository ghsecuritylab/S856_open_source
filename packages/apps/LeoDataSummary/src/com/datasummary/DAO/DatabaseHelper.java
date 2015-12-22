package com.datasummary.DAO;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteDatabase.CursorFactory;
import com.datasummary.unit.LeoDataSummaryLog;

  

public class DatabaseHelper extends SQLiteOpenHelper {
	public static String TAG="DatabaseHelper";
	private static final int VERSION = 1;
	private static final String DBNAME="ratetest.db";
	public DatabaseHelper(Context context, String name, CursorFactory factory,
			int version) {
		super(context, name, factory, version);
		// TODO Auto-generated constructor stub
	}
	public DatabaseHelper(Context context){
		this(context,DBNAME,VERSION);
	}
	public DatabaseHelper(Context context,String name,int version){
		this(context, DBNAME,null,version);
	}

	@Override
	public void onCreate(SQLiteDatabase db) {
		// TODO Auto-generated method stub
		LeoDataSummaryLog.d(TAG,"create a Database");
		db.execSQL("create table total(id INTEGER primary key AUTOINCREMENT,imsi varchar(20),sub int,rx decimal,tx decimal,date date)");
		db.execSQL("create table detail(id INTEGER primary key AUTOINCREMENT,uid int,rx decimal,tx decimal,date varchar(20))");
		db.execSQL("create table current(id INTEGER primary key AUTOINCREMENT,imsi varchar(20),sub int,rx decimal,tx decimal,date date)");
	}

	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
		// TODO Auto-generated method stub
		LeoDataSummaryLog.d(TAG,"update a Database");
	}

}
