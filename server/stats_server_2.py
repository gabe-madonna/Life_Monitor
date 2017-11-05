import records
import math
import json
import datetime
import gmplot_string2
import random
from datetime import datetime
from datetime import timedelta
import os
import time
#import numpy as np
#from bokeh.plotting import figure, output_file, show
import numpy
from bokeh.plotting import figure, output_file, show
from bokeh.embed import components, file_html
from bokeh.resources import CDN
from twilio.rest import Client

def color_path(points, thresholds):
    appended = False
    paths = [[] for i in range(len(thresholds) + 1)]
    for i in range(len(points) - 1):
        appended = False
        for j in range(len(thresholds)):
            if numpy.linalg.norm(points[i] - points[i + 1]) < thresholds[j]:
                paths[j] += [([points[i][0], points[i+1][0]], [points[i][1], points[i+1][1]])]
                appended = True
                break
        if not appended:
            paths[-1].append(([points[i][0], points[i+1][0]], [points[i][1], points[i+1][1]]))
        #for i in range(len(paths)):
            #print("path" , i, "length", len(paths[i]))
    return paths


def get_map_html(paths, colors, gmap):
    for i in range(len(paths)):
        for segment in paths[i]:
            gmap.plot(segment[0], segment[1], colors[i], edge_width=2 *  i + 2)
    return gmap.draw("test_map.html")

def get_scope(x,y):
    lat_arg = -32.992335
    lon_arg = -60.652066
    lat_chi = 32.552604
    lon_chi = 119.717577

    dist = math.sqrt((lat_arg-lat_chi)**2 + (lon_arg-lon_chi)**2)

    if(len(x)>0 and len(y)>0):
        new_dist = math.sqrt((max(x)-min(x))**2 + (max(y)-min(y))**2)
        if new_dist == 0:
            return 16
        return 1/(new_dist/dist*28)
    else:
        return 1

def get_distance(lat1, lon1, lat2, lon2):
    radius = 6378137
    lat_diff = math.radians(lat1-lat2)
    lon_diff = math.radians(lon1-lon2)
    lat_1 = math.radians(lat1)
    lat_2 = math.radians(lat2)

    a = math.sin(lat_diff/2)**2 + math.cos(lat_1)*math.cos(lat_2)*math.sin(lon_diff/2)**2
    c = 2*math.asin(math.sqrt(a))
    d = radius*c
    return d

def get_total_path(lats, lons):
    distance = 0
    l = len(lats)
    for i in range(len(lats)):
        distance+=get_distance(lats[i%l], lons[i%l], lats[(i+1)%l], lons[(i+1)%l])
    return distance

def web_handler(request):

    method = request['REQUEST_METHOD']

    db = records.Database('mysql://student:6s08student@localhost/iesc')

    if method == "POST":
        parameters = request['POST']

        keys = [key for key in parameters.keys()]
        kerberos = parameters['kerberos']
        # return keys
        everytime = ['kerberos', 'heart_rate', 'lat', 'lon', 'steps']
        every_val = []
        for e in everytime:
            every_val.append(parameters[e])
        db.query("INSERT into activity_stats (kerberos, heart_rate, lat, lon, steps) VALUES (:kerb, :h, :lat_, :lon_, :s)",
                kerb=every_val[0], h = every_val[1], lat_ =every_val[2], lon_=every_val[3], s= every_val[4])
        print("inserted into database: activity_stats!")
        for e in everytime:
            keys.remove(e)
        # return keys
        # for e in keys:
        rows = db.query("SELECT * from user_stats ORDER BY ID DESC")
        rows = rows.as_dict()
        #print(rows)
        row_to_change = {}
        for row in rows:
            #print (row)
            if kerberos in row.values():
                row_to_change = row
                break
        print(row_to_change)
        if len(row_to_change)>0:
            for e in keys:
                row_to_change[e] = parameters[e]
            db.query("INSERT into user_stats (kerberos, height, weight, age, phone_number, gender, goals_weight, goals_calories, goals_distance, goals_steps) VALUES (:k, :hi, :we, :age_, :phone, :gen, :g_w, :g_c, :g_d, :g_s)",
                k=kerberos, hi=row_to_change['height'], we=row_to_change['weight'], age_=row_to_change['age'], phone=row_to_change['phone_number'], gen=row_to_change['gender'], g_w=row_to_change['goals_weight'],
                g_c=row_to_change['goals_calories'], g_d=row_to_change['goals_distance'], g_s=row_to_change['goals_steps'])
            return "updated into: user stats, for key " + e + " vaalue " + parameters[e] + " for keberos " + kerberos

        else:
            db.query("INSERT into user_stats (kerberos, height, weight, age, phone_number, gender, goals_weight, goals_calories, goals_distance, goals_steps) VALUES (:k, 0, 0, 0, 0, 0, 0, 0, 0, 0)", k = kerberos)
            # db.query("INSERT into user_stats (kerberos, height, weight, age, gender, goals_weight, goals_calories, goals_distance, goals_steps) VALUES (:k, 0, 0, 0, 0, 0, 0, 0, 0, 0)", k = kerberos)
            print("created a new entry with kerberos = " + kerberos)
            rows = db.query("SELECT * from user_stats WHERE kerberos = :k", k = kerberos)
            row_to_change = rows.as_dict()[0]
            for e in keys:
                row_to_change[e] = parameters[e]
                print("inserted into: user stats, for key " + e + " vaalue " + parameters[e] + " for keberos " + kerberos)
            return "created new entry for " + kerberos



    elif method == "GET":
        parameters = request['GET']
        if 'all' in parameters:
            rows = db.query('SELECT * from activity_stats ORDER BY ID DESC')
            return rows.as_dict()
        elif 'stats' in parameters:
            rows = db.query('SELECT * from user_stats ORDER BY ID DESC')
            return rows.as_dict()
        elif 'steps' in parameters:
            rows = db.query('SELECT kerberos, steps, `time` FROM activity_stats ORDER BY ID DESC')
            dict_rows = rows.as_dict()
            right_list = []
            for x in dict_rows:
                d = {}
                for key in x.keys():
                    if x[key] is not None:
                        if x[key] !=0:
                            d[key] = x[key]
                if len(d) >0:
                    if 'steps' in d:
                        right_list.append(d)
            return right_list
        elif 'date' in parameters:
            rows = db.query("SELECT * from activity_stats WHERE time>= '2017-5-2 00:00:00'")
            return rows.as_dict()

        elif 'goals' in parameters:
            rows = db.query("SELECT * from user_stats")
            dict_rows = rows.as_dict()
            goals_weight = dict_rows[0]['goals_weight']
            goals_calories = dict_rows[0]['goals_calories']
            goals_steps = dict_rows[0]['goals_steps']
            goals_distance = dict_rows[0]['goals_distance']
            return "weight=" + goals_weight + "##" + "distance=" + goals_distance + "##" + "calories=" + goals_calories + "##" + "steps=" + goals_steps

        elif 'period' in parameters and 'start' in parameters and 'data' in parameters:
            #dd/mm/yyyy
            os.environ['TZ'] = 'UTC+0'
            time.tzset()
            kerberos = parameters['kerberos']
            period = parameters['period']
            start = parameters['start'].split("/")
            date_1 = "%s-%s-%s" %(start[2], start[0], str(int(start[1])+1))
            date_start = datetime.strptime("%s-%s-%s" %(start[2], start[0], start[1]) + " 00:00:00", "%Y-%m-%d %H:%M:%S")
            #print(date_1)
            data = parameters['data']
            height = int(parameters['height'])
            width = int(parameters['width'])
            interval = "INTERVAL %d %s" %(1, period)
            if period == "DAY":
                date_end = date_start + timedelta(days=1)
                rows = db.query("SELECT * from activity_stats WHERE time >= DATE_SUB(:d, INTERVAL 1 DAY) AND time <=:d AND kerberos = :k ORDER BY ID DESC", d=date_1, i= interval, s=data, k=kerberos)
            elif period == "WEEK":
                date_end = date_start + timedelta(days=7)
                rows = db.query("SELECT * from activity_stats WHERE time >= DATE_SUB(:d, INTERVAL 1 WEEK) AND time <=:d AND kerberos = :k ORDER BY ID DESC", d=date_1, i= interval, s=data, k=kerberos)
            elif period == "MONTH":
                # date_end = date_start - timedelta(months=1)
                date_end = datetime(date_start.year, date_start.month+1, date_start.day, 0, 0, 0)
                rows = db.query("SELECT * from activity_stats WHERE time >= DATE_SUB(:d, INTERVAL 1 MONTH) AND time <=:d AND kerberos = :k ORDER BY ID DESC", d=date_1, i= interval, s=data, k=kerberos)
            elif period == "YEAR":
                #date_end = date_start - timedelta(year=1)
                date_start = datetime(2017, 1, 1, 0, 0, 0)
                date_end = datetime(2017, 12, 31, 0, 0, 0)
                rows = db.query("SELECT * from activity_stats WHERE time >= DATE_SUB(:d, INTERVAL 1 YEAR) AND time <=:d AND kerberos = :k ORDER BY ID DESC", d=date_1, i= interval, s=data, k=kerberos)
            rows_dict = rows.as_dict()
            x = []
            y = []
            for a in rows_dict:
                x.append(a['time'])
                y.append(a[data])
            p = figure(title="Data for " + data.lower() + " by " + period.lower(), x_axis_label="time", y_axis_label=data, x_axis_type = "datetime", width=width, height=height, x_range=(date_start.timestamp()*1000, date_end.timestamp()*1000))
            if (len(x) <1):
                p.line(0,0,legend=period, line_width = 1)
            p.line(x,y,legend = period, line_width = 3)
            p.toolbar_location = None

            html = file_html(p, CDN, "my_plot")
            return html

        elif 'period' in parameters and 'start' in parameters and 'path' in parameters:
            kerberos = parameters['kerberos']
            period = parameters['period']
            start = parameters['start'].split("/")
            date_1 = "%s-%s-%s" %(start[2], start[0], str(int(start[1])+1))

            interval = "INTERVAL %d %s" %(1, period)

            if kerberos != "all":
                rows = db.query("SELECT * from activity_stats WHERE time >= DATE_SUB(:d, INTERVAL 1 DAY) AND time <=:d AND kerberos = :k ORDER BY ID DESC", d=date_1, i= interval, k=kerberos)
            else:
                rows = db.query("SELECT * from activity_stats WHERE time >= DATE_SUB(:d, INTERVAL 1 DAY) AND time <=:d ORDER BY ID DESC", d=date_1, i= interval)
            rows_dict = rows.as_dict()

            lats = []
            lons = []

            for a in rows_dict:
                lats.append(a['lat'])
                lons.append(a['lon'])
            lat0 = numpy.average(lats)
            lon0 = numpy.average(lons)

            for i in range(len(lats)):
                if lat[i] == str(360)


            zoom = get_scope(lats, lons)

            if (len(lats)>0 and len(lons)>0):
                gmap = gmplot_string2.GoogleMapPlotter(lat0, lon0, zoom)
                points = list(map(numpy.array, list(zip(lats, lons))))

                thresholds = [.003,0.0045]
                colors = ["red", "yellow","green", ]
                paths = color_path(points, thresholds)
            else:
                gmap = gmplot_string2.GoogleMapPlotter(42.359850, -71.0991060, 16)
                lats = [42.359850]
                lons = [-71.0991060]
                points = list(map(numpy.array, list(zip(lats, lons))))

                thresholds = [.003,0.0045]
                colors = ["red", "yellow","green", ]
                paths = color_path(points, thresholds)

            return get_map_html(paths, colors, gmap)

        elif 'goal' in parameters and 'kerberos' in parameters and 'date' in parameters:
            kerberos = parameters['kerberos']
            goal = parameters['goal']
            date = parameters['date']
            goals_rows = db.query("SELECT * from activity_stats WHERE kerberos = :k ORDER BY ID DESC LIMIT 1", k = kerberos)
            goals_row = goals_rows.as_dict()
            if goal == "weight":
                weight_goal = goals_row[0]['goals_weight']
                weight = goals_row[0]['weight']
                return weight + "#" + weight_goal
            else:
                rows = db.query("SELECT * from activity_stats WHERE kerberos = :k ORDER BY ID DESC LIMIT 1", k = kerberos)
                row = rows.as_dict()
                if goal == "steps":
                    step_goals = goals_row[0]['goals_steps']
                    steps = rows[0]['steps']
                    return steps + "#" + step_goals
                if goal == "calories":
                    calories_goals = goals_row[0]['goals_calories']
                    calories = 1000
                    return calories + "#" + calories_goals
                if goal == "distance":
                    date_1 = "%s-%s-%s" %(start[2], start[0], str(int(start[1])+1))
            # goals_weight, goals_calories, goals_distance, goals_steps
            return


        elif 'kerberos' in parameters and 'alarm' in parameters:
            account_sid = "ACa10a9537cc3abfb2253a130a47071cf5"
            auth_token  = "1922c298bef586e74aad3159a3c7bacc"
            client = Client(account_sid, auth_token)
            message = client.messages.create(
            #    to="+19787991436",
                to = "+19787991436",
                from_="+19788505028",
                body="pramoda suckssssss\n wait...what?")
            return "message sent to " + "+19787991436" + " id: " + message.sid


        #period, start, type
        #period = day, week, month, year
        #type
