#!/usr/bin/octave
format bank
f1=dlmread('data/The one I need/F1710176-2018-06-29-14-03-16/F1710176-2018-06-29-14-03-16.txt','',8,0);
%f2=dlmread('data/LOG-2018-06-27-22-31-09/F1691030-2018-06-27-22-31-09/F1691030-2018-06-27-22-31-09.txt','',8,0);
span=dlmread('data/The one I need/SPAN-2018-06-29-14-03-16/SPAN-2018-06-29-14-03-16.txt',',');
span_gps_sec=span(:,11);
span_time=round(span_gps_sec*1000);
span_lat=span(:,12);
span_lon=span(:,13);
span_alt=span(:,14);
span_roll=span(:,18);
span_pitch=span(:,19);
span_heading=span(:,20);
f1_ms_gps=f1(:,28);
f1_time=round(f1_ms_gps);
f1_heading=f1(:,1);
f1_pitch=f1(:,2);
f1_roll=f1(:,3);
f1_lat=f1(:,16);
f1_lon=f1(:,17);
f1_alt=f1(:,18);
%f2_heading=f2(:,1);
%f2_pitch=f2(:,2);
%f2_roll=f2(:,3);
%f2_lat=f2(:,16);
%f2_lon=f2(:,17);
%f2_alt=f2(:,18);

%Concatenating for tables 
t1=[f1_time,f1_heading, f1_pitch, f1_roll, f1_lat, f1_lon, f1_alt];
%t2=[f2_time,f2_heading, f2_pitch, f2_roll, f2_lat, f2_lon, f2_alt];
t_span=[span_time,span_heading, span_pitch, span_roll, span_lat, span_lon, span_alt];

%array_intersect A is f1 and B is span
[~, ind_ins, ind_span]=intersect(round(f1_time/5)*5,span_time);
f1=t1(ind_ins,:);
span=t_span(ind_span,:);
%f1 and span are time intersected tables
%Now, find the index you need to start evaluating the test from)

span_lat=span(:,5);
for i=1:length(span_lat)
   if span_lat(i)~=0
        break;
    end
end

f1=f1(i:end,:);
span=span(i:end,:);
clear i 

%Reassigning parameters:
f1_time=f1(:,1);
f1_heading=f1(:,2);
f1_pitch=f1(:,3);
f1_roll=f1(:,4);
f1_lat=f1(:,5);
f1_lon=f1(:,6);
f1_alt=f1(:,7);
span_time=span(:,1);
span_heading=span(:,2);
span_pitch=span(:,3);
span_roll=span(:,4);
span_lat=span(:,5);
span_lon=span(:,6);
span_alt=span(:,7);;

%Check if this is still applicable 
f1_alt=f1_alt-33.5;
%poserr 
radius_earth=6371000;
pos1=[f1_lat, f1_lon, f1_alt];
pos2=[span_lat, span_lon, span_alt];
delta=zeros(size(pos1));
delta(:,1) = radius_earth.*sin(pi/180.*(pos1(:,1) -pos2(:,1)));
delta(:,2) = radius_earth * sin(pi/180*(pos1(:,2) - pos2(:,2))).* cos(pi/180*(pos1(:,1) - pos2(:,1)));
delta(:,3) = pos1(:,3)-pos2(:,3);

Result = table;
Result.Time = span_time;
Result.Minutes = (Result.Time - Result.Time(1))/60000;
Result.INS_lat = f1_lat;
Result.INS_lon = f1_lon;
Result.INS_alt = f1_alt;
Result.SPAN_lat = span_lat;
Result.SPAN_lon = span_lon;
Result.SPAN_alt = span_alt;
Result.err_lat = delta(:,1);
Result.err_lon = delta(:,2);
Result.err_alt = delta(:,3);

%atterr 
Att1=[f1_heading, f1_pitch, f1_roll];
Att2=[span_heading,span_pitch, span_roll];
clear pi
delta=180/pi*asin(sin(pi/180*(Att2-Att1)));
delta (1:3,:)









