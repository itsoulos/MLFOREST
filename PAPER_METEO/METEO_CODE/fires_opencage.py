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

df=df.head(5)

from time import sleep

from opencage.geocoder import OpenCageGeocode

# Initialize the OpenCage geocoder with your API key
api_key = '298bc9e547e74b58adbf388945a3fa2f'
geocoder = OpenCageGeocode(api_key)


def get_lat_long(row):
    # Construct the full address from available fields
    location_query = f"{row['address']}, {row['area']}, {row['municipality']}, Ελλάδα" if pd.notna(row['area']) else f"{row['address']}, {row['municipality']}, Ελλάδα"
    print(location_query)
    try:
        # Perform the geocoding request
        result = geocoder.geocode(location_query)
        if result:
            location = result[0]['geometry']
            return pd.Series([location['lat'], location['lng']])
        else:
            location_query = f"{row['area']}, {row['municipality']} or '',Ελλάδα" if pd.notna(row['area']) else f"{row['municipality']}, Ελλάδα"
            print("trying new: " + location_query)
            result = geocoder.geocode(location_query)
            if result:
                location = result[0]['geometry']
                return pd.Series([location['lat'], location['lng']])
            else:
                location_query = f"{row['municipality']}, Ελλάδα" 
                print("trying new again: " + location_query)
                result = geocoder.geocode(location_query)
                if result:
                    location = result[0]['geometry']
                    return pd.Series([location['lat'], location['lng']])
                else:        
                    return pd.Series([None, None])
    except Exception as e:
        print(f"Error: {e}")
        return pd.Series([None, None])


# Apply the function to each row
df[['latitude', 'longtitude']] = df.apply(get_lat_long, axis=1)

# Display the updated DataFrame
print(df)


from tkinter.filedialog import askdirectory
import os
directory = askdirectory(title="Select Directory to Save Excel File")
file_path = os.path.join(directory, "result.xlsx")
    
# Save the DataFrame to Excel
df.to_excel(file_path, index=False)
