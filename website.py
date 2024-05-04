# web
import RPi.GPIO as GPIO
import time

# using raspberry pi GPIO pins
GPIO.setmode(GPIO.BCM)
from flask import Flask, request, render_template_string, jsonify, session
from flask_session import Session

global_input = None
new_state = None

app = Flask(__name__)
app.config['SECRET_KEY'] = 'your_secret_key'  # Needed for session management
app.config['SESSION_TYPE'] = 'filesystem'  # Sessions will be stored in the filesystem
Session(app)

# HTML template of the web application
# website refreshes every minute
# session data is stored to track the current state of the relay
HTML = '''
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>farmnet</title>
    <link rel="stylesheet" href="{{ url_for('static', filename='style.css') }}">
    <h1> Sensor readings </h1>
    <p> Soil emperature: {{ temperature }}C </p>
    <p> Water level: {{ water_level }}mm </p>
    <h1> Relay operation </h1>
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.6.0/jquery.min.js"></script>
    <script>
        $(document).ready(function() {
            $('#toggleButton').click(function() {
                var currentState = $(this).val();
                $.ajax({
                    type: 'POST',
                    url: '/toggle',
                    data: JSON.stringify({state: currentState}),
                    contentType: 'application/json;charset=UTF-8',
                    success: function(response) {
                        $('#toggleButton').html(response.button_text);
                        $('#toggleButton').val(response.button_value);
                        $('#stateDisplay').text('Current State: ' + response.button_value);
                    }
                });
            });
        });
    </script>
    <meta http-equiv="refresh" content="60">
</head>
<body>
    <h3> Relay toggle </h3>
    <button id="toggleButton" value="{{ button_value }}">{{ button_text }}</button>
    <p id="stateDisplay">Current State: {{ button_value }}</p>
</body>
</html>
'''


# python flask framework is used to create the web application
def process_input(input_data):
    # Example function that could do more complex processing
    print("Processed input: ", input_data)
    GPIO.setup(24, GPIO.OUT)
    if input_data == 'On':
        GPIO.output(24, GPIO.HIGH)
    elif input_data == 'Off':
        GPIO.output(24, GPIO.LOW)

    # Further processing code here


# code for displaying the temperature and water level sensor data on the web application
@app.route('/', methods=['GET', 'POST'])
def home():
    global global_input
    dataT = []
    dataWL = []
    with open('temperature.txt', 'r') as file:
        for line in file:
            clean_line = line.strip()
            temperature = clean_line
    with open('water_level.txt', 'r') as file:
        for line in file:
            clean_line = line.strip()
            water_level = str(int((int(clean_line) / 82)))
    if 'button_value' not in session:
        # Initialize the session with default values if not already set
        session['button_value'] = 'Off'
        session['button_text'] = 'Turn On'
    return render_template_string(HTML, temperature=temperature, water_level=water_level,
                                  button_value=session['button_value'], button_text=session['button_text'])


# code for the toggle button for operating the relay
@app.route('/toggle', methods=['POST'])
def toggle():
    global new_state
    current_state = request.json['state']
    if current_state == 'Off':
        new_state = 'On'
        session['button_value'] = 'On'
        session['button_text'] = 'Turn Off'
        process_input(new_state)
    else:
        new_state = 'Off'
        session['button_value'] = 'Off'
        session['button_text'] = 'Turn On'
        process_input(new_state)
    return jsonify({'button_text': session['button_text'], 'button_value': session['button_value']})


# the ip address is given to us from Tailscale
if __name__ == '__main__':
    app.run(host='<your ip address or url>', port=8082, debug=True)
    while True:
        process_input(global_input)



