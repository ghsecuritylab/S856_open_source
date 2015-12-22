package com.datasummary.model;

import java.sql.Date;



public class TotalInfo {
	private int id;
	private String imsi;
	private int sub;
	private double rx;
	private double tx;
	private Long date;
	public TotalInfo(int tid,String timsi,int tsub,double trx,double ttx,Long tdate){
		id=tid;
		imsi=timsi;
		sub=tsub;
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
	public String getImsi() {
		return imsi;
	}
	public void setImsi(String imsi) {
		this.imsi = imsi;
	}
	public int getSub() {
		return sub;
	}
	public void setSub(int sub) {
		this.sub = sub;
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
	
	public Long getDate() {
		return date;
	}
	public void setDate(Long date) {
		this.date = date;
	}
	@Override
	public String toString() {
		return "TotalInfo [id=" + id + ", imsi=" + imsi + ", sub=" + sub
				+ ", rx=" + rx + ", tx=" + tx + ", date=" + date + "]";
	}
	
}
