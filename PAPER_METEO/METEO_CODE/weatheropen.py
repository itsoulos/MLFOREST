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

def is_long_fire(datetime1, datetime2):
    # Calculate the absolute difference between the two datetimes
    time_difference = abs(datetime1 - datetime2)
    
    # Check if the difference is greater than 24 hours
    return 1 if time_difference.total_seconds() > 24 * 3600 else 0
    
#############################
import openmeteo_requests

import requests_cache
import pandas as pd
from retry_requests import retry
import requests

row = df.iloc[0]

def get_weather_info(row,output_path):
    print('new run')
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
    try:
        responses = openmeteo.weather_api(url, params=params)
    
    
        # Process first location. Add a for-loop for multiple locations or weather models
        response = responses[0]
        #############################################################################################
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
        hourly_data["start_temperature_2m"] = hourly_temperature_2m
        hourly_data["start_relative_humidity_2m"] = hourly_relative_humidity_2m
        hourly_data["start_dew_point_2m"] = hourly_dew_point_2m
        hourly_data["start_precipitation"] = hourly_precipitation
        hourly_data["start_weather_code"] = hourly_weather_code
        hourly_data["start_cloud_cover"] = hourly_cloud_cover
        hourly_data["start_et0_fao_evapotranspiration"] = hourly_et0_fao_evapotranspiration
        hourly_data["start_vapour_pressure_deficit"] = hourly_vapour_pressure_deficit
        hourly_data["start_wind_speed_10m"] = hourly_wind_speed_10m
        hourly_data["start_wind_speed_100m"] = hourly_wind_speed_100m
        hourly_data["start_wind_direction_10m"] = hourly_wind_direction_10m
        hourly_data["start_wind_direction_100m"] = hourly_wind_direction_100m
        
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
        daily_data["start_daylight_duration"] = daily_daylight_duration
        daily_data["start_sunshine_duration"] = daily_sunshine_duration
        hourly_data_row['start_daily_daylight_duration']=daily_daylight_duration.item()
        hourly_data_row['start_daily_sunshine_duration']=daily_sunshine_duration.item()
        hourly_data_row=hourly_data_row.drop('date')
        
        ######################################################################################
        
        if is_long_fire(row["start_date"], row["end_date"]):
            params = {
            	"latitude": row['latitude'],
            	"longitude": row['longtitude'],
            	"start_date": row['end_date'].date().strftime('%Y-%m-%d'),#CHANGED PARAMETERS
            	"end_date": row['end_date'].date().strftime('%Y-%m-%d'),#CHANGED PARAMETERS
            	"hourly": ["temperature_2m", "relative_humidity_2m", "dew_point_2m", "precipitation", "weather_code", "cloud_cover", "et0_fao_evapotranspiration", "vapour_pressure_deficit", "wind_speed_10m", "wind_speed_100m", "wind_direction_10m", "wind_direction_100m"],
            	"daily": ["daylight_duration", "sunshine_duration"],
            	"timezone": "auto"
            }
            ############### REDO THE REQUEST WITH CHANGED PARAMETERS
            responses = openmeteo.weather_api(url, params=params)
        
        
            # Process first location. Add a for-loop for multiple locations or weather models
            response = responses[0]
            #############################################################################################
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
            hourly_data["end_temperature_2m"] = hourly_temperature_2m
            hourly_data["end_relative_humidity_2m"] = hourly_relative_humidity_2m
            hourly_data["end_dew_point_2m"] = hourly_dew_point_2m
            hourly_data["end_precipitation"] = hourly_precipitation
            hourly_data["end_weather_code"] = hourly_weather_code
            hourly_data["end_cloud_cover"] = hourly_cloud_cover
            hourly_data["end_et0_fao_evapotranspiration"] = hourly_et0_fao_evapotranspiration
            hourly_data["end_vapour_pressure_deficit"] = hourly_vapour_pressure_deficit
            hourly_data["end_wind_speed_10m"] = hourly_wind_speed_10m
            hourly_data["end_wind_speed_100m"] = hourly_wind_speed_100m
            hourly_data["end_wind_direction_10m"] = hourly_wind_direction_10m
            hourly_data["end_wind_direction_100m"] = hourly_wind_direction_100m
            
            hourly_dataframe = pd.DataFrame(data = hourly_data)
            i=find_closest_datetime_index(hourly_dataframe, 'date', row['end_datetime'])
            hourly_data_row_part2=hourly_dataframe.iloc[i]
        
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
            daily_data["end_daylight_duration"] = daily_daylight_duration
            daily_data["end_sunshine_duration"] = daily_sunshine_duration
            hourly_data_row_part2['end_daily_daylight_duration']=daily_daylight_duration.item()
            hourly_data_row_part2['end_daily_sunshine_duration']=daily_sunshine_duration.item()
            hourly_data_row_part2=hourly_data_row_part2.drop('date')
            
            ######################################################################################
        else:
            #DONT REDO THE REQUEST, JUST SAME DATA AS START
            response = responses[0]
            #############################################################################################
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
            hourly_data["end_temperature_2m"] = hourly_temperature_2m
            hourly_data["end_relative_humidity_2m"] = hourly_relative_humidity_2m
            hourly_data["end_dew_point_2m"] = hourly_dew_point_2m
            hourly_data["end_precipitation"] = hourly_precipitation
            hourly_data["end_weather_code"] = hourly_weather_code
            hourly_data["end_cloud_cover"] = hourly_cloud_cover
            hourly_data["end_et0_fao_evapotranspiration"] = hourly_et0_fao_evapotranspiration
            hourly_data["end_vapour_pressure_deficit"] = hourly_vapour_pressure_deficit
            hourly_data["end_wind_speed_10m"] = hourly_wind_speed_10m
            hourly_data["end_wind_speed_100m"] = hourly_wind_speed_100m
            hourly_data["end_wind_direction_10m"] = hourly_wind_direction_10m
            hourly_data["end_wind_direction_100m"] = hourly_wind_direction_100m
            
            hourly_dataframe = pd.DataFrame(data = hourly_data)
            i=find_closest_datetime_index(hourly_dataframe, 'date', row['end_datetime'])
            hourly_data_row_part2=hourly_dataframe.iloc[i]
        
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
            daily_data["end_daylight_duration"] = daily_daylight_duration
            daily_data["end_sunshine_duration"] = daily_sunshine_duration
            hourly_data_row_part2['end_daily_daylight_duration']=daily_daylight_duration.item()
            hourly_data_row_part2['end_daily_sunshine_duration']=daily_sunshine_duration.item()
            hourly_data_row_part2=hourly_data_row_part2.drop('date')
            ######################################################################################
        combined_row = pd.concat([hourly_data_row, hourly_data_row_part2])

        return combined_row
    except requests.exceptions.HTTPError as e:
        if e.response.status_code == 429:
            print("Rate limit exceeded. Saving progress and stopping.")
            df.to_excel(output_path, index=False)
            raise SystemExit("API request limit reached. Execution stopped.")
        else:
            print(f"An HTTP error occurred: {e}")
            raise
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        raise

new_rows= [
    "start_temperature_2m",
    "start_relative_humidity_2m",
    "start_dew_point_2m",
    "start_precipitation",
    "start_weather_code",
    "start_cloud_cover",
    "start_et0_fao_evapotranspiration",
    "start_vapour_pressure_deficit",
    "start_wind_speed_10m",
    "start_wind_speed_100m",
    "start_wind_direction_10m",
    "start_wind_direction_100m",
    "start_daily_daylight_duration",
    "start_daily_sunshine_duration",
    "end_temperature_2m",
    "end_relative_humidity_2m",
    "end_dew_point_2m",
    "end_precipitation",
    "end_weather_code",
    "end_cloud_cover",
    "end_et0_fao_evapotranspiration",
    "end_vapour_pressure_deficit",
    "end_wind_speed_10m",
    "end_wind_speed_100m",
    "end_wind_direction_10m",
    "end_wind_direction_100m",
    "end_daily_daylight_duration",
    "end_daily_sunshine_duration"
]

import os
directory = os.path.dirname(file_path)
new_file_path = os.path.join(directory, "weather_output_file.xlsx")

df[new_rows] = df.apply(lambda row: get_weather_info(row,new_file_path), axis=1)

df.to_excel(new_file_path, index=False)