# -*- coding: utf-8 -*-

website = """

<!DOCTYPE html>
<html lang="en">
<head>
  <!--declare div tags with specific titles as variables to be accessed by javascript code-->
    <div id = "user_current" class = "vars" title = "gdolphin"></div>
    <div id = "user_gdoplphin" class = "vars user_names" title = "gdolphin"></div>
    <div id = "user_pkarnati" class = "vars user_names" title = "pkarnati"></div>
    <div id = "user_gmadonna" class = "vars user_names" title = "gmadonna"></div>
    <div id = "user_dimitral" class = "vars user_names" title = "dimitral"></div>

    <div id = date_day  class = vars title = 1></div>
    <div id = date_month class = vars title = 1></div>
    <div id = date_year class = vars title = 2000></div>
    <div id = date_period class = vars title = "day"></div>
    <div id = stat_type class = vars title = "heartrate"></div>

    <div id = date_day2 class = vars title = 1></div>
    <div id = date_month2 class = vars title = 1></div>
    <div id = date_year2 class = vars title = 2000></div>

  <meta charset="utf-8">
  <title>fitness_page</title>
</head>

<body>
    <script>

        function switch_user() {
          document.getElementById("myDropdown").classList.toggle("show");
        }

        function update_stat_graph() {
          var kerberos = document.getElementById("user_current").title;
          var stat_type = document.getElementById("stat_type").title;
            var day = document.getElementById("date_day").title;
            var month = document.getElementById("date_month").title;
            var year = document.getElementById("date_year").title;
            var period = document.getElementById("date_period").title;
            var w = parseInt(.92 * document.getElementById("stat_graph").offsetWidth, 10);
            var h = parseInt(.90 * document.getElementById("stat_graph").offsetHeight, 10);

            if (stat_type == "heartrate") {stat_type = "heart_rate";}

            if (day < 10) {
              day = '0' + day;
            }
            if (month < 10) {
              month = '0' + month;
            }

            var request = "http://iesc-s2.mit.edu/6S08dev/pkarnati/final/sb2.py?" +
            "kerberos=" + kerberos +
            "&period=" + period.toUpperCase() +
            "&start=" + month + "/" + day + "/" + year +
            "&data=" + stat_type +
            "&width=" + w +
            "&height=" + h;
            document.getElementById("stat_graph").src = request;
        }

          function update_path_graph() {
            var kerberos = document.getElementById("user_current").title;
            var day = document.getElementById("date_day2").title;
            var month = document.getElementById("date_month2").title;
            var year = document.getElementById("date_year2").title;

            var request = "http://iesc-s2.mit.edu/6S08dev/pkarnati/final/sb2.py?" +
            "kerberos=" + kerberos +
            "&period=DAY" +
            "&start=" + month + "/" + day + "/" + year +
            "&path=1"
            document.getElementById("path_graph").src = request;
          }

        function changePeriod(period) {
            document.getElementById("date_period").title = period;
            if (period == 'month') {
              document.getElementById('date_day').title = 1;
            }
            else if (period == 'year') {
              document.getElementById('date_day').title = 1;
              document.getElementById('date_month').title = 1;
            }
            changeDate('0', 'stats');
        }

        function change_stat(stat_type) {
            document.getElementById("stat_type").title = stat_type;
            document.getElementById("stat_label").innerHTML = stat_type;
        }

        function changeDate(d, side) {

            var months = {
                1: ["January", 31],
                2: ["February", 28],
                3: ["March", 31],
                4: ["April", 30],
                5: ["May", 31],
                6: ["June", 30],
                7: ["July", 31],
                8: ["August", 31],
                9: ["September", 30],
                10: ["October", 31],
                11: ["November", 30],
                12: ["December", 31]};

            d = parseInt(d);

            if (side == 'path') {
                var period = 'day';
                var day = document.getElementById("date_day2").title;
                var month = document.getElementById("date_month2").title;
                var year = document.getElementById("date_year2").title;
            }
            else {
                var day = document.getElementById("date_day").title;
                var month = document.getElementById("date_month").title;
                var year = document.getElementById("date_year").title;
                var period = document.getElementById("date_period").title;
            }

            if (period == 'day') {day = parseInt(day) + d;}
            else if (period == 'week') {day = parseInt(day) + 7*d;}
            else if (period == 'month') {month = parseInt(month) + d;}
            else if (period == 'year') {year = parseInt(year) + d;}

            if ((period == 'day' || period == 'week') && day > months[month][1])
                {
                    day = day - months[month][1];
                    month++;
                }
            else if (day < 1) {
                month--;
                if (month < 1) {day = months[month + 12][1] + day;}
                else {day = months[month][1] + day;}
            }
            if (month > 12) {
                month -= 12;
                year++;
            }
            else if (month < 1) {
                year--;
                month += 12;
            }

            if (side == 'path') {
                document.getElementById("date_day2").title = day;
                document.getElementById("date_month2").title = month;
                document.getElementById("date_year2").title = year;
            }
            else {
                document.getElementById("date_day").title = day;
                document.getElementById("date_month").title = month;
                document.getElementById("date_year").title = year;
            }

            var date_string = months[month][0] + " " + day + ", " + year;

            if (side == 'path') {
                document.getElementById("path_graph_label").innerHTML =
                  date_string;}
            else {
                if (period == 'week') {
                    date_string = "Week of " + date_string;}
                else if (period == 'month') {
                    date_string = months[month][0] + ", " + year;}
                else if (period == 'year') {
                    date_string = year;}
                document.getElementById("stat_graph_label").innerHTML =
                  date_string;
            }
          }

          function gen_users() {
              var user_names = document.querySelectorAll(".user_names");
              for (i = 0; i < user_names.length; i++) {
                var name = document.createElement("a");
                var text = user_names[i].title;
                var node = document.createTextNode(text);
                name.appendChild(node);
                name.setAttribute("class", "username_list_element");
                name.setAttribute("onclick", "change_user('" + text + "');");

                document.getElementById("myDropdown").appendChild(name);
              }
          }

          function change_user(user) {
              document.getElementById("user_current").title = user;
              document.getElementById("user_title").innerHTML =
                  document.getElementById("user_current").title + "'s Activity";
              update_path_graph();
              update_stat_graph();
          }
          function initialize() {
              var today = new Date();
              var day = today.getDate();
              var month = (today.getMonth() + 1);
              var year = today.getFullYear();
              document.getElementById("user_title").innerHTML =
                  document.getElementById("user_current").title + "'s Activity";
              document.getElementById("date_day").title = day;
              document.getElementById("date_month").title = month;
              document.getElementById("date_year").title = year;
              document.getElementById("date_day2").title = day;
              document.getElementById("date_month2").title = month;
              document.getElementById("date_year2").title = year;
              changeDate('0', 'path');
              changeDate('0', 'stats');
              update_path_graph();
              update_stat_graph();
              gen_users();
            }
    </script>
    <style>
        #page_header {
          display: flex;
          flex-flow: row nowrap;
        }
        #dropdown {
          align-self: flex-end;
        }
        #user_dropdown_balance {
          align-self: flex-end;
          color: transparent;
        }
        .username_list_element{
            cursor:pointer;
        }
        .dropbtn {
            background-color: lightgray;
            color: black;
            padding: 16px;
            font-size: 20px;
            border: none;
            cursor: pointer;
        }

        /* Dropdown button on hover & focus */
        .dropbtn:hover{
            background-color: white;
        }
        /* The container <div> - needed to position the dropdown content */
        .dropdown {
            position: relative;
            display: inline-block;
        }
        /* Dropdown Content (Hidden by Default) */
        .dropdown-content {
            display: none;
            position: absolute;
            background-color: #f9f9f9;
            font-size: 20px;
            box-shadow: 0px 8px 16px 0px rgba(0,0,0,0.2);
            z-index: 1;
        }
        /* Links inside the dropdown */
        .dropdown-content a {
            color: black;
            padding: 12px 16px;
            text-decoration: none;
            display: block;
        }
        /* Change color of dropdown links on hover */
        .dropdown-content a:hover {background-color: #f1f1f1}
        /* Show the dropdown menu (use JS to add this class to the .dropdown-content container when the user clicks on the dropdown button) */
        .show {display:block;}
        h1 {
            width: 80vw;
            text-align: center;
            padding: 7px;
            background-color: transparent;
            color: white;
            box-sizing: border-box;
            margin: 0px;
            font-size: 7.7vh;
            font-family: monospace;
            max-height: 10vh;
        }
        body {
            display: flex;
            justify-content: center;
            box-sizing: border-box;
            flex-flow: row wrap;
            min-height: 90vh;
            background-color: gray;
        }
        .page_divide {
            display: flex;
            flex-flow: column nowrap;
            box-sizing: border-box;
            background-color: lightgray;
            padding: 10px;
            flex: 1 1;
            margin: 7.5px;
            max-height: 83vh;
        }
        h2 {
            flex: 0 0;
            text-align:center;
            background-color: transparent;
            font-size: 6vh;
            margin: 10px;
            font-family: monospace;
            }
        .graph_header {
            display:flex;
            flex: 0 .1;
            flex-flow: row nowrap;
            justify-content: space-between;
            background-color: transparent;
            box-sizing: border-box;
            padding: 10px;
            margin: 10px 0px;
            align-items: center;
        }
        h3 {
            font-size: 2.25vw;
            font-weight: 600;
            align-self:center;
            font-family: monospace;
            background-color: lightcyan;
            border-radius: 5px;
            padding: 0 5px;
            white-space: nowrap;
        }
        .btn_box {
            margin-top: 10px;
            display: flex;
            flex-flow: column nowrap;
            width: 12vw;
        }

        #graph_interface {
            display: flex;
            flex-flow: row nowrap;
            justify-content: space-between;
        }
        #stat_graph {
            flex: 2 1;
            align-self: stetch;
            width:40vw;
            padding-top: 10px;
        }

        .nav_btn {
            background-color: lightgray;
            border: 2px lightgray solid;
            border-radius: 3px;
            color: transparent;
            background-size: contain;
            background-repeat: no-repeat;
            background-position: center;
            cursor:pointer;
            padding: 0px 3px;
            min-height: 5vh;
        }
        .stats_btn {
            box-sizing: border-box;
            background-color: lightgray;
            padding: 10px;
            text-align: center;
            font-size: 1.7vw;
            font-family: monospace;
            color: dimgray;
            flex: 1 1 87px;
            align-items: stretch;
            cursor:pointer;
            border: none;
        }
        .nav_btn:hover  {
            background-color: darkgray;
            border-color: darkgray;
        }
        .stats_btn:hover  {
            background-color: darkgray;
            color:cyan;
        }
        #stat_btn_left {
            background-image:
                url("https://upload.wikimedia.org/wikipedia/commons/thumb/1/16/TriangleArrow-Left.svg/461px-TriangleArrow-Left.svg.png");
        }
        #stat_btn_right {
            background-image:
                url("https://upload.wikimedia.org/wikipedia/commons/thumb/2/24/TriangleArrow-Right.svg/461px-TriangleArrow-Right.svg.png");
        }
        #path_btn_left {
            background-image:
                url("https://upload.wikimedia.org/wikipedia/commons/thumb/1/16/TriangleArrow-Left.svg/461px-TriangleArrow-Left.svg.png");
        }
        #path_btn_right {
            background-image:
                url("https://upload.wikimedia.org/wikipedia/commons/thumb/2/24/TriangleArrow-Right.svg/461px-TriangleArrow-Right.svg.png");
        }
        iframe {
            margin-top: 10px;
            flex:1;
            padding: 5px;
            border: none;
        }
    </style>
    <div id = "page_header">
      <div id = "user_dropdown_balance" >Change User</div>

      <h1 id = "user_title">User's Fitness</h1>
      <div class="dropdown">
        <button onclick="switch_user()" class="dropbtn">Switch User </button>
        <div id="myDropdown" class="dropdown-content">
        </div>
      </div>
    </div>

    <div id = "left_page" class = page_divide>
        <h2 id = "stat_label">heartrate</h2>

        <div id = stat_graph_header class = "graph_header">
            <button type= button id = stat_btn_left class = nav_btn onclick = "changeDate('-1', 'stats'); update_stat_graph();">
            OO</button>

            <h3 id = stat_graph_label>Date</h3>

            <button type= button id = stat_btn_right class = nav_btn onclick = "changeDate('1', 'stats'); update_stat_graph();">
               OO</button>
        </div>

        <div id = graph_interface>

            <div id = "period_btn_box" class = "btn_box">
                <button type= button id = "day_btn" class = "stats_btn"
                        onclick="changePeriod('day'); update_stat_graph();">Day</button>
                <button type= button id = week_btn class = stats_btn
                        onclick="changePeriod('week'); update_stat_graph();">Week</button>
                <button type= button id = month_btn class = stats_btn
                        onclick="changePeriod('month'); update_stat_graph();">Month</button>
                <button type= button id = year_btn class = stats_btn
                        onclick="changePeriod('year'); update_stat_graph();">Year</button>
            </div>

            <iframe id = stat_graph src = "https://answers.squarespace.com/storage/temp/1387-screen-shot-2016-04-14-at-125043.png">
            </iframe>

            <div id = "stat_btn_box" class = btn_box>
                <button type= button id = heart_btn class = stats_btn onclick = "change_stat('heartrate'); update_stat_graph();">heartrate</button>
                <button type= button id = steps_btn class = stats_btn onclick = "change_stat('steps'); update_stat_graph();">steps</button>
                <button type= button id = calories_btn class = stats_btn onclick = "change_stat('calories'); update_stat_graph();">calories</button>
                <button type= button id = distance_btn class = stats_btn onclick = "change_stat('distance'); update_stat_graph();">distance</button>
            </div>
        </div>
    </div>

    <div id = "right_page" class = page_divide>
        <h2 id = "path_label">Path</h2>
        <div id = path_graph_header class = graph_header>

            <button type= button id =path_btn_left class = nav_btn onclick = "changeDate('-1', 'path'); update_path_graph();">
            OO</button>

            <h3 id = path_graph_label>Date</h3>

            <button type= button id = path_btn_right class = nav_btn onclick = "changeDate('1', 'path'); update_path_graph();">
               OO</button>
        </div>

        <iframe  id = "path_graph" src="https://answers.squarespace.com/storage/temp/1387-screen-shot-2016-04-14-at-125043.png">
        <p>Your browser does not support iframes.</p>
        </iframe>
        <script> initialize(); </script>
    </div>
</body>
</html>




"""


def web_handler(request):
    method = request['REQUEST_METHOD']
    if method == "GET":
        return website
