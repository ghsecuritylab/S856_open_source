package com.datasummary.DAO;

import java.util.ArrayList;

import com.datasummary.model.DetailInfo;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;


public class DetailDAO {

	private DatabaseHelper helper;
	private SQLiteDatabase db;
  
	public DetailDAO(Context context) {
		helper = new DatabaseHelper(context);

	}  

	public void add(DetailInfo detailInfo) {
		db = helper.getWritableDatabase();
		db.execSQL(
				"insert into detail (uid,rx,tx,date) values (?,?,?,?)",
				new Object[] { detailInfo.getUid(), detailInfo.getRx(),
						detailInfo.getTx(), detailInfo.getDate() });
	}

	public ArrayList<DetailInfo> findbydate(String date) {

		db = helper.getReadableDatabase();
		Cursor cursor = db
				.query("detail", null, "date=?", new String[]{
						date
				}, null, null, null);

		if (cursor != null) {

			ArrayList<DetailInfo> detailInfos = new ArrayList<DetailInfo>();

			while (cursor.moveToNext()) {
				DetailInfo tmpDetailInfo = new DetailInfo(cursor.getInt(cursor
						.getColumnIndex("id")),  cursor.getInt(cursor
						.getColumnIndex("uid")), cursor.getDouble(cursor
						.getColumnIndex("rx")), cursor.getDouble(cursor
						.getColumnIndex("tx")), cursor.getString(cursor
						.getColumnIndex("date")));
				detailInfos.add(tmpDetailInfo);
			}

			return detailInfos;
		} else {
			return null;
		}

	}


}
