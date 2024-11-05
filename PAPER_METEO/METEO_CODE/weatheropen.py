# -*- coding: utf-8 -*-
"""
Created on Mon Nov  4 16:08:41 2024

@author: AndreasMiltiadous
"""

import pandas as pd
from tkinter import Tk
from tkinter.filedialog import askopenfilename

# Hide the root Tkinter window
Tk().withdraw()

# Open file dialog to select an .xlsx file
file_path = askopenfilename(filetypes=[("Excel files", "*.xlsx")])

# Check if a file was selected
if file_path:
    # Read the selected Excel file into a DataFrame
    df = pd.read_excel(file_path, header=0)  # header=0 uses the first row as column names
    print("DataFrame loaded successfully!")
    print(df.head())  # Display the first few rows of the DataFrame

else:
    print("No file selected.")
    
df['start_datetime']=pd.to_datetime(df['start_date'].dt.strftime('%Y-%m-%d') + ' ' + df['start_time'])
df['end_datetime']=pd.to_datetime(df['end_date'].dt.strftime('%Y-%m-%d') + ' ' + df['end_time'])
    
###############################


def find_closest_datetime_index(df, column_name, target_datetime):
    # Ensure both target_datetime and the DataFrame column are timezone-naive
    if target_datetime.tzinfo is not None:
        target_datetime = target_datetime.tz_convert(None)

    # Convert the specified column to datetime if it's not already
    df[column_name] = pd.to_datetime(df[column_name], errors='coerce').dt.tz_localize(None)
    
    # Calculate the absolute difference between each datetime in the column and the target datetime
    time_diffs = abs(df[column_name] - target_datetime)
    
    # Find the index of the row with the minimum time difference
    closest_index = time_diffs.idxmin()
    
    return closest_index
    
#############################
import openmeteo_requests

import requests_cache
import pandas as pd
from retry_requests import retry

row = df.iloc[0]

def get_weather_info(row):
    # Setup the Open-Meteo API client with cache and retry on error
    cache_session = requests_cache.CachedSession('.cache', expire_after = -1)
    retry_session = retry(cache_session, retries = 5, backoff_factor = 0.2)
    openmeteo = openmeteo_requests.Client(session = retry_session)
    
    # Make sure all required weather variables are listed here
    # The order of variables in hourly or daily is important to assign them correctly below
    url = "https://archive-api.open-meteo.com/v1/archive"
    params = {
    	"latitude": row['latitude'],
    	"longitude": row['longtitude'],
    	"start_date": row['start_date'].date().strftime('%Y-%m-%d'),
    	"end_date": row['start_date'].date().strftime('%Y-%m-%d'),
    	"hourly": ["temperature_2m", "relative_humidity_2m", "dew_point_2m", "precipitation", "weather_code", "cloud_cover", "et0_fao_evapotranspiration", "vapour_pressure_deficit", "wind_speed_10m", "wind_speed_100m", "wind_direction_10m", "wind_direction_100m"],
    	"daily": ["daylight_duration", "sunshine_duration"],
    	"timezone": "auto"
    }
    responses = openmeteo.weather_api(url, params=params)


    # Process first location. Add a for-loop for multiple locations or weather models
    response = responses[0]
    
    # Process hourly data. The order of variables needs to be the same as requested.
    hourly = response.Hourly()
    hourly_temperature_2m = hourly.Variables(0).ValuesAsNumpy()
    hourly_relative_humidity_2m = hourly.Variables(1).ValuesAsNumpy()
    hourly_dew_point_2m = hourly.Variables(2).ValuesAsNumpy()
    hourly_precipitation = hourly.Variables(3).ValuesAsNumpy()
    hourly_weather_code = hourly.Variables(4).ValuesAsNumpy()
    hourly_cloud_cover = hourly.Variables(5).ValuesAsNumpy()
    hourly_et0_fao_evapotranspiration = hourly.Variables(6).ValuesAsNumpy()
    hourly_vapour_pressure_deficit = hourly.Variables(7).ValuesAsNumpy()
    hourly_wind_speed_10m = hourly.Variables(8).ValuesAsNumpy()
    hourly_wind_speed_100m = hourly.Variables(9).ValuesAsNumpy()
    hourly_wind_direction_10m = hourly.Variables(10).ValuesAsNumpy()
    hourly_wind_direction_100m = hourly.Variables(11).ValuesAsNumpy()
    
    hourly_data = {"date": pd.date_range(
    	start = pd.to_datetime(hourly.Time(), unit = "s", utc = True),
    	end = pd.to_datetime(hourly.TimeEnd(), unit = "s", utc = True),
    	freq = pd.Timedelta(seconds = hourly.Interval()),
    	inclusive = "left"
    )}
    hourly_data["temperature_2m"] = hourly_temperature_2m
    hourly_data["relative_humidity_2m"] = hourly_relative_humidity_2m
    hourly_data["dew_point_2m"] = hourly_dew_point_2m
    hourly_data["precipitation"] = hourly_precipitation
    hourly_data["weather_code"] = hourly_weather_code
    hourly_data["cloud_cover"] = hourly_cloud_cover
    hourly_data["et0_fao_evapotranspiration"] = hourly_et0_fao_evapotranspiration
    hourly_data["vapour_pressure_deficit"] = hourly_vapour_pressure_deficit
    hourly_data["wind_speed_10m"] = hourly_wind_speed_10m
    hourly_data["wind_speed_100m"] = hourly_wind_speed_100m
    hourly_data["wind_direction_10m"] = hourly_wind_direction_10m
    hourly_data["wind_direction_100m"] = hourly_wind_direction_100m
    
    hourly_dataframe = pd.DataFrame(data = hourly_data)
    i=find_closest_datetime_index(hourly_dataframe, 'date', row['start_datetime'])
    hourly_data_row=hourly_dataframe.iloc[i]

    # Process daily data. The order of variables needs to be the same as requested.
    daily = response.Daily()
    daily_daylight_duration = daily.Variables(0).ValuesAsNumpy()
    daily_sunshine_duration = daily.Variables(1).ValuesAsNumpy()
    
    daily_data = {"date": pd.date_range(
    	start = pd.to_datetime(daily.Time(), unit = "s", utc = True),
    	end = pd.to_datetime(daily.TimeEnd(), unit = "s", utc = True),
    	freq = pd.Timedelta(seconds = daily.Interval()),
    	inclusive = "left"
    )}
    daily_data["daylight_duration"] = daily_daylight_duration
    daily_data["sunshine_duration"] = daily_sunshine_duration
    hourly_data_row['daily_daylight_duration']=daily_daylight_duration.item()
    hourly_data_row['daily_sunshine_duration']=daily_sunshine_duration.item()
    hourly_data_row=hourly_data_row.drop('date')
    return hourly_data_row

new_rows= [
    "temperature_2m",
    "relative_humidity_2m",
    "dew_point_2m",
    "precipitation",
    "weather_code",
    "cloud_cover",
    "et0_fao_evapotranspiration",
    "vapour_pressure_deficit",
    "wind_speed_10m",
    "wind_speed_100m",
    "wind_direction_10m",
    "wind_direction_100m",
    "daily_daylight_duration",
    "daily_sunshine_duration"
]

df[new_rows] = df.apply(get_weather_info, axis=1)
import os
directory = os.path.dirname(file_path)
new_file_path = os.path.join(directory, "weather_output_file.xlsx")
df.to_excel(new_file_path, index=False)