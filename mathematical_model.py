#!/usr/bin/env python
# coding: utf-8

# In[1]:


import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from sysidentpy.model_structure_selection import FROLS
from sysidentpy.basis_function import Polynomial
from sysidentpy.utils.generate_data import get_miso_data, get_siso_data
from sysidentpy.metrics import root_relative_squared_error
from sysidentpy.utils.plotting import plot_residues_correlation, plot_results


# Create 100 dummy values for temperature (as exogenous input)
np.random.seed(0)  # Seed for reproducibility
temperature = np.random.uniform(low=10, high=35, size=100)  # Temperature in degrees Celsius

# Create 100 dummy values for soil moisture (as the auto-regressive part)
soil_moisture = 0.75 - 0.02 * temperature + np.random.normal(0, 0.05, size=temperature.shape)

# Create the data frame
data = pd.DataFrame({'temperature': temperature, 'soil_moisture': soil_moisture})

#No. of lags for the ARX model
max_lag = 2  

# Preparing the data for System Identification using SysIdentPy
x = data[['temperature']].values
y = data[['soil_moisture']].values

# Fit the ARX model using FROLS algorithm for system identification
model = FROLS(
    order_selection=True, #If True, the algorithm tests different model orders and selects the one that fits best.
    n_info_values=10, #when order_selection is true, this parameter defines how many different model orders the algorithm will consider during the order selection process.
    extended_least_squares=False, 
    ylag=max_lag,  # number of past values of y to include
    xlag=max_lag,  # number of past values of x to include
    info_criteria='bic',#uses bayesian information criterion
    estimator= "least_squares", #uses least squares method 
    basis_function=Polynomial(degree=1)  # Specify basis function here if your library version supports it
)

# Fit the model
model.fit(X=x, y=y)

# Getting the estimated parameters
estimated_coefficients = model.theta
print('Estimated Coefficients:\n', estimated_coefficients)

# Getting the model estimation based on the identified model
yhat = model.predict(X=x, y=y)

# Calculating the error
rrse = root_relative_squared_error(y, yhat)
print(f'RRSE: {rrse}')

# Set a globally recognized and valid style
plt.style.use('seaborn-v0_8-white')

# Plot the results using a valid style
plot_results(y=y, yhat=yhat, n=100, style='seaborn-v0_8-white')
#fig.savefig('model_results.png')  # Save the figure to a file


# In[ ]:




