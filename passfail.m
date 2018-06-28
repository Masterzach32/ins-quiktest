#!/usr/bin/octave
format bank
f1=dlmread('data/LOG-2018-06-28-20-19-48/F1710176-2018-06-28-20-19-48/F1710176-2018-06-28-20-19-48.txt','',8,0);
%f2=dlmread('data/LOG-2018-06-27-22-31-09/F1691030-2018-06-27-22-31-09/F1691030-2018-06-27-22-31-09.txt','',8,0);
span=dlmread('data/LOG-2018-06-28-20-19-48/SPAN-2018-06-28-20-19-48/SPAN-2018-06-28-20-19-48.txt',',');
size(span)
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


for i=1:length(span_lat)
   if span_lat(i)~=0
        break;
    end
end

f1=f1(i:end,:);
span=span(i:end,:);
clear i 
