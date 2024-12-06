from datetime import datetime, timedelta
import calendar
from typing import OrderedDict
import os

months_temp = [-11.6, -7.6, -1.0, 6., 10.7, 14.5, 18.8, 20.6, 16.7, 9.6, -0.1, -8.8]

def get_month_temp(t_month): # 0-11 == Jan-Dec
    month = int(t_month)
    start_month_temp = months_temp[month]
    end_month_temp = months_temp[0 if month > 10 else month + 1]
    
    t_this_month = t_month - month

    return start_month_temp * (1 - t_this_month) + end_month_temp * t_this_month

def get_day_temp(t_day):
    if (t_day < 0.25):
        return -4 - 2*(t_day*4)
    elif (t_day < 0.5):
        return -6 + 14 * (t_day - 0.25)*4
    elif (t_day < 0.75):
        return 8 - 4 * (t_day - 0.5)*4
    else:   
        return 4 - 8 * (t_day - 0.75)*4
    

today = datetime.now()
sec_ago = today - timedelta(seconds=1)
year_ago = today.replace(year=today.year - 1)
month_ago = today - timedelta(days=30)

year_log = OrderedDict()

year_gen_date = year_ago
while (year_gen_date < today):
    last_day = calendar.monthrange(year_gen_date.year, year_gen_date.month)[-1]
    t_month = (year_gen_date.month - 1) + (year_gen_date.day - 1) / last_day
    
    year_log[int(datetime.timestamp(year_gen_date))] = get_month_temp(t_month)
    year_gen_date += timedelta(days=1)

log_dir = "./build/logs"
if not os.path.exists(log_dir):
    os.makedirs(log_dir)

with open(log_dir+"/day.log", "w") as f:
    for d, t in year_log.items():
        f.writelines(f"{d} {t:.5f}\n")


month_log = OrderedDict()

DAY_SEC = 60 * 60 * 24
month_gen_date = month_ago
while (month_gen_date < today):
    last_day = calendar.monthrange(month_gen_date.year, month_gen_date.month)[-1]
    t_month = (month_gen_date.month - 1) + (month_gen_date.day - 1) / last_day
 
    midnight = month_gen_date.replace(hour=0, minute=0, second=0, microsecond=0)
    t_day = (month_gen_date - midnight).seconds / DAY_SEC
    
    month_log[int(datetime.timestamp(month_gen_date))] = get_month_temp(t_month) + get_day_temp(t_day)
    month_gen_date += timedelta(hours=1)

with open(log_dir+"/hour.log", "w") as f:
    for d, t in month_log.items():
        f.writelines(f"{d} {t:.5f}\n")