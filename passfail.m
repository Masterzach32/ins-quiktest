#!/usr/bin/octave-cli

# passfail.m

format bank
args = argv();

INS_filename = 'sample/LOG-2018-07-04-18-22-16/F1691030-2018-07-04-18-22-16/F1691030-2018-07-04-18-22-16.txt';
SPAN_filename = 'sample/LOG-2018-07-04-18-22-16/SPAN-2018-07-04-18-22-16/SPAN-2018-07-04-18-22-16.txt';
out_filename='data/Results.csv';

if length(args) == 3
    INS_filename = args{1};
    SPAN_filename = args{2};
    out_filename = args{3};
end

f1=dlmread(INS_filename,'',8,0);
span=dlmread(SPAN_filename,',');
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
% f2_heading=f2(:,1);
% f2_pitch=f2(:,2);
% f2_roll=f2(:,3);
% f2_lat=f2(:,16);
% f2_lon=f2(:,17);
% f2_alt=f2(:,18);

% Concatenating for tables
t1=[f1_time,f1_heading, f1_pitch, f1_roll, f1_lat, f1_lon, f1_alt];
%t2=[f2_time,f2_heading, f2_pitch, f2_roll, f2_lat, f2_lon, f2_alt];
t_span=[span_time,span_heading, span_pitch, span_roll, span_lat, span_lon, span_alt];

% array_intersect A is f1 and B is span
[~, ind_ins, ind_span]=intersect(round(f1_time/5)*5,span_time);
f1=t1(ind_ins,:);
span=t_span(ind_span,:);
% f1 and span are time intersected tables
% Now, find the index you need to start evaluating the test from)

span_lat=span(:,5);
for i=1:length(span_lat)
   if span_lat(i)~=0
        break;
    end
end

f1=f1(i:end,:);
span=span(i:end,:);
clear i

% Reassigning parameters:
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
span_alt=span(:,7);

% Check if this is still applicable
% f1_alt=f1_alt-33.5;

% poserr
radius_earth=6371000;
pos1=[f1_lat, f1_lon, f1_alt];
pos2=[span_lat, span_lon, span_alt];
delta=zeros(size(pos1));
delta(:,1) = radius_earth.*sin(pi/180.*(pos1(:,1) -pos2(:,1)));
delta(:,2) = radius_earth * sin(pi/180*(pos1(:,2) - pos2(:,2))).* ...
    cos(pi/180*(pos1(:,1) - pos2(:,1)));
delta(:,3) = pos1(:,3)-pos2(:,3);

Result_Time = span_time;
Result_Minutes = (Result_Time - Result_Time(1))/60000;
Result_INS_lat = f1_lat;
Result_INS_lon = f1_lon;
Result_INS_alt = f1_alt;
Result_SPAN_lat = span_lat;
Result_SPAN_lon = span_lon;
Result_SPAN_alt = span_alt;
Result_err_lat = delta(:,1);
Result_err_lon = delta(:,2);
Result_err_alt = delta(:,3);

%atterr
Att1=[f1_heading, f1_pitch, f1_roll];
Att2=[span_heading,span_pitch, span_roll];
clear pi
delta=180/pi*asin(sin(pi/180*(Att2-Att1)));

Result_INS_heading = f1_heading;
Result_INS_pitch =f1_pitch;
Result_INS_roll = f1_roll;
Result_SPAN_heading = span_heading;
Result_SPAN_pitch = span_pitch;
Result_SPAN_roll = span_roll;
Result_err_heading = delta(:,1);
Result_err_pitch = delta(:,2);
Result_err_roll = delta(:,3);
clear delta;

Result=[Result_Time, Result_Minutes, Result_INS_lat,Result_INS_lon, Result_INS_alt,...
 Result_SPAN_lat, Result_SPAN_lon, Result_SPAN_alt,...
 Result_err_lat, Result_err_lon, Result_err_alt,...
Result_INS_heading, Result_INS_pitch, Result_INS_roll,...
Result_SPAN_heading, Result_SPAN_pitch, Result_SPAN_roll,...
Result_err_heading, Result_err_pitch, Result_err_roll
];

fileID = fopen(out_filename,'w');
fprintf(fileID,'Time,Minutes,INS_Lat,INS_Lon,INS_alt,SPAN_lat,SPAN_long,SPAN_alt,Err_lat,Err_lon,Err_alt,INS_heading,INS_pitch,INS_roll,SPAN_heading,SPAN_pitch,SPAN_roll,Err_heading,Err_pitch,Err_roll\n');
for i=1:length(Result)
    fprintf(fileID, '%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n',Result(i,:));
end
fclose(fileID);
