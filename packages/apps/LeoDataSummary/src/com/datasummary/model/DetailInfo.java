package com.datasummary.model;

public class DetailInfo {

	private int id;
	private int uid;
	private double rx;
	private double tx;
	private String date;
	public DetailInfo(int tid,int tuid,double trx,double ttx,String tdate){
		id=tid;
		uid=tuid;
		rx=trx;
		tx=ttx;
		date=tdate;
		
		
	}
	public int getId() {
		return id;
	}
	public void setId(int id) {
		this.id = id;
	}
	
	public int getUid() {
		return uid;
	}
	public void setUid(int uid) {
		this.uid = uid;
	}
	public double getRx() {
		return rx;
	}
	public void setRx(double rx) {
		this.rx = rx;
	}
	public double getTx() {
		return tx;
	}
	public void setTx(double tx) {
		this.tx = tx;
	}
	public String getDate() {
		return date;
	}
	public void setDate(String date) {
		this.date = date;
	}
	@Override
	public String toString() {
		return "DetailInfo [id=" + id + ", uid=" + uid + ", rx=" + rx + ", tx="
				+ tx + ", date=" + date + "]";
	}
	

}
