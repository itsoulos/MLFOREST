#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Oct 25 14:41:39 2024

@author: andreasmiltiadous
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
    


#keep only 50 first for trying


from time import sleep

from opencage.geocoder import OpenCageGeocode
from opencage.geocoder import RateLimitExceededError

# Initialize the OpenCage geocoder with your API key

api_keys=['298bc9e547e74b58adbf388945a3fa2f',
          'f2803657ebff4df4a46b07f8e464c987',
          '0f693a7d633f46d0b8b19a8ed8e8d6c4',
          '42c49da01f1343a8a9a8efd8a0e344fd',
          '802d4b2996f24fbdb49a9770673064ec',
          '793b5b7b0a594409a08b6f3508483a24']



def api_key_rotator(api_keys):
    """
    Generator that cycles through a list of API keys. 
    Returns the next API key on each call, or 1 if all keys are exhausted.
    """
    while True:
        for key in api_keys:
            yield key
        yield 1  # Return 1 if no more API keys are available


api_key_generator = api_key_rotator(api_keys)
current_api_key = next(api_key_generator)
geocoder = OpenCageGeocode(current_api_key)


def get_lat_long(row, df, output_path):
    
    global current_api_key, geocoder
    
    # Construct the full address from available fields
    location_query = f"{row['address']}, {row['area']}, {row['municipality']}, Ελλάδα" if pd.notna(row['area']) else f"{row['address']}, {row['municipality']}, Ελλάδα"
    print(location_query)
    try:
        # Perform the geocoding request
        result = geocoder.geocode(location_query)
        sleep(1)
        if result:
            location = result[0]['geometry']
            print("a row is done")
            return pd.Series([location['lat'], location['lng']])
        else:
            location_query = f"{row['area']}, {row['municipality']} or '',Ελλάδα" if pd.notna(row['area']) else f"{row['municipality']}, Ελλάδα"
            print("trying new: " + location_query)
            result = geocoder.geocode(location_query)
            sleep(1)
            if result:
                location = result[0]['geometry']
                print("a row is done")
                return pd.Series([location['lat'], location['lng']])
            else:
                location_query = f"{row['municipality']}, Ελλάδα" 
                print("trying new again: " + location_query)
                result = geocoder.geocode(location_query)
                sleep(1)
                if result:
                    location = result[0]['geometry']
                    print("a row is done")
                    return pd.Series([location['lat'], location['lng']])
                else:        
                    return pd.Series([None, None])
    except RateLimitExceededError:
        print("Rate limit exceeded. Switching to the next API key.")
        # Get the next API key
        current_api_key = next(api_key_generator)
        if current_api_key == 1:
            print("All API keys exhausted. Saving progress and stopping.")
            # Save the partially processed DataFrame
            df.to_excel(output_path, index=False)
            raise SystemExit("All API keys have been used. Stopping further requests.")
            
        geocoder = OpenCageGeocode(current_api_key)
        # Retry the function for the same row with the new key
        return get_lat_long(row, df, output_path)
    except Exception as e:
        # Catch any other unexpected errors, save progress, and exit
        print(f"An unexpected error occurred: {e}")
        print("Saving progress and exiting.")
        df.to_excel(output_path, index=False)
        raise SystemExit("Exiting due to an unexpected error.")
            
from tkinter.filedialog import askdirectory
import os
directory = askdirectory(title="Select Directory to Save Excel File")
output_path = os.path.join(directory, "result.xlsx")


# Apply the function to each row
df[['latitude', 'longtitude']] = df.apply(lambda row: get_lat_long(row, df, output_path), axis=1)

    
# Save the DataFrame to Excel
df.to_excel(output_path, index=False)
