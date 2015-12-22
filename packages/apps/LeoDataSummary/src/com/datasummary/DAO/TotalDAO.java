package com.datasummary.DAO;

import java.sql.Date;
import java.util.ArrayList;

import com.datasummary.model.TotalInfo;
import com.datasummary.unit.LeoDataSummaryLog;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.content.ContentValues;

public class TotalDAO {
	private DatabaseHelper helper;
	private SQLiteDatabase db;
	private static double test = 2.0;
	public static String TAG="TotalDAO";
	public TotalDAO(Context context) {
		helper = new DatabaseHelper(context);

	}

	public void add(TotalInfo totalInfo) {
		db = helper.getWritableDatabase();
		db.execSQL(
				"insert into total (imsi,sub,rx,tx,date) values (?,?,?,?,?)",
				new Object[] {totalInfo.getImsi(),
						totalInfo.getSub(), totalInfo.getRx(),
						totalInfo.getTx(), totalInfo.getDate() });
	}

	public void update(long date,double rx, double tx) {
		db = helper.getWritableDatabase();
		ContentValues cv = new ContentValues();
		cv.put("date",date);
		cv.put("rx",rx);
		cv.put("tx",tx);
		db.update("total",cv,"date = ?", new String[]{Long.toString(date)});
	}

	public void addCurrentTable(TotalInfo totalInfo) {
		db = helper.getWritableDatabase();
		db.execSQL(
				"insert into current (imsi,sub,rx,tx,date) values (?,?,?,?,?)",
				new Object[] {totalInfo.getImsi(),
						totalInfo.getSub(), totalInfo.getRx(),
						totalInfo.getTx(), totalInfo.getDate() });
	}

	public void updateCurrentTable(long date,double rx, double tx) {
		db = helper.getWritableDatabase();
		ContentValues cv = new ContentValues();
		cv.put("date",date);
		cv.put("rx",rx);
		cv.put("tx",tx);
		db.update("current",cv,"date = ?", new String[]{Long.toString(date)});
	}
	public void deleteCurrentTable(long date) {
		db = helper.getWritableDatabase();
		db.delete("current","date = ?", new String[]{Long.toString(date)});
	}	
	public boolean ifExistCurrentTable(long date, int sub, String imsi) {
		LeoDataSummaryLog.d(TAG,"ifExistDate date = "+date);
		LeoDataSummaryLog.d(TAG,"ifExistDate sub = "+sub);
		LeoDataSummaryLog.d(TAG,"ifExistDate imsi = "+imsi);
		db = helper.getReadableDatabase();
		Cursor cursor = db.rawQuery("SELECT * FROM current WHERE date =? AND sub =? AND imsi =?",
			new String[]{Long.toString(date),String.valueOf(sub), imsi});
		LeoDataSummaryLog.d(TAG,"ifExistDate cursor.getCount() = "+cursor.getCount());
		if(cursor.getCount() > 0) {
			cursor.close();
			return true;
		}else {
			cursor.close();
			return false;
		}
	}
	public boolean ifExistDate(long date) {
		db = helper.getReadableDatabase();
		Cursor cursor = db.rawQuery("select * from total where date = ?",new String[]{Long.toString(date)});
		LeoDataSummaryLog.d(TAG,"ifExistDate cursor.getCount() = "+cursor.getCount());
		if(cursor.getCount() > 0) {
			cursor.close();
			return true;
		}else {
			cursor.close();
			return false;
		}
	}
	public ArrayList<TotalInfo> findbetweendateCurrentTable(String imsi,int sub,String date1,String date2) {

		db = helper.getReadableDatabase();
		Cursor cursor = db.rawQuery("select * from current where imsi=? and sub=? and date between ? and ? order by date",new String[]{
				imsi,String.valueOf(sub),date1,date2
		});

		if (cursor != null) {

			ArrayList<TotalInfo> totalInfos = new ArrayList<TotalInfo>();

			while (cursor.moveToNext()) {
				TotalInfo tmptotalinfo = new TotalInfo(cursor.getInt(cursor
						.getColumnIndex("id")), cursor.getString(cursor
						.getColumnIndex("imsi")), cursor.getInt(cursor
						.getColumnIndex("sub")), cursor.getDouble(cursor
						.getColumnIndex("rx")), cursor.getDouble(cursor
						.getColumnIndex("tx")), cursor.getLong(cursor.getColumnIndex("date")));
				totalInfos.add(tmptotalinfo);
			}
			cursor.close();
			return totalInfos;
		} else {
			cursor.close();
			return null;
		}

	}
	
	public ArrayList<TotalInfo> findbetweendate(String imsi,int sub,String date1,String date2) {

		db = helper.getReadableDatabase();
		Cursor cursor = db.rawQuery("select * from total where imsi=? and sub=? and date between ? and ? order by date",new String[]{
				imsi,String.valueOf(sub),date1,date2
		});

		if (cursor != null) {

			ArrayList<TotalInfo> totalInfos = new ArrayList<TotalInfo>();

			while (cursor.moveToNext()) {
				TotalInfo tmptotalinfo = new TotalInfo(cursor.getInt(cursor
						.getColumnIndex("id")), cursor.getString(cursor
						.getColumnIndex("imsi")), cursor.getInt(cursor
						.getColumnIndex("sub")), cursor.getDouble(cursor
						.getColumnIndex("rx")), cursor.getDouble(cursor
						.getColumnIndex("tx")), cursor.getLong(cursor.getColumnIndex("date")));
				totalInfos.add(tmptotalinfo);
			}
			cursor.close();
			return totalInfos;
		} else {
			cursor.close();
			return null;
		}

	}

}
