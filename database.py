import psycopg2
import time
from datetime import datetime
import random

# Your database credentials
username = "<your username>"
password = "<your password>"
db_connection = "<your database name>.postgres.database.azure.com:5432/postgres?sslmode=require"


# The database entry code is run inside a function which is then called infinitely
# The data is being read from the text files we created using esp32 reading code
# electrical conductivity is just a random value being sent to the database for fun
def data_poll():
    CONNECTION = f"postgres://{username}:{password}@{db_connection}"
    with open('temperature.txt', 'r') as file:
        for line in file:
            clean_line = line.strip()
            temperature = clean_line
    with open('water_level.txt', 'r') as file:
        for line in file:
            clean_line = line.strip()
            humidity = str(int((int(clean_line) / 82)))

    data = {
        'timestamp': datetime.now(),
        'temp': temperature,
        'moist': humidity,
        'elecon': round(random.uniform(1, 5), 2)  # example electrical conductivity
    }

    with psycopg2.connect(CONNECTION) as conn:
        cur = conn.cursor()

        # this is the sql data entry
        sql_str = "INSERT INTO <your table name> ( timestamp, temp, moist, elecon) VALUES ( %s, %s, %s, %s)"
        cur.execute(sql_str, (data["timestamp"], data["temp"], data["moist"], data["elecon"]))

        conn.commit()


count = 0

while True:
    count = count + 1
    print(count)
    data_poll()
    # this function runs every 60 seconds i.e. the time frame is one minute, samples for each datapoint
    time.sleep(60)


