from flask import Flask, request, jsonify
import logging
from tabulate import tabulate

app = Flask(__name__)

# log for the matrixs
logging.basicConfig(filename='matrices_log_ui.txt', level=logging.INFO, format='%(message)s')

# log for the Timestamps
timestamp_log = logging.getLogger('timestamp')
timestamp_log.setLevel(logging.INFO)
timestamp_handler = logging.FileHandler('timestamp_log_ui.txt')
timestamp_handler.setLevel(logging.INFO)
timestamp_handler.setFormatter(logging.Formatter('%(message)s'))
timestamp_log.addHandler(timestamp_handler)

def log_matrix_to_file(matrix):
    logging.info(matrix)
    logging.info("") 

def log_timestamp_to_file(timestamp):
    timestamp_log.info(timestamp)

@app.route('/receive_matrix', methods=['POST'])
def receive_matrix():
    matrix_data = request.json
    print(matrix_data)
    
    if 'originalMatrix' in matrix_data and 'transformedMatrix' in matrix_data:
        original_matrix_table = tabulate(matrix_data['originalMatrix'], tablefmt="fancy_grid")
        transformed_matrix_table = tabulate(matrix_data['transformedMatrix'], tablefmt="fancy_grid")
        
        log_matrix_to_file("Original Matrix:")
        log_matrix_to_file(original_matrix_table)
        
        log_matrix_to_file("Transformed Matrix:")
        log_matrix_to_file(transformed_matrix_table)
        
        return jsonify({"message": "Matrices received and logged successfully"}), 200
    else:
        return jsonify({"error": "Invalid data format. Expected 'originalMatrix' and 'transformedMatrix' keys."}), 400

@app.route('/receive_timestamp', methods=['POST'])
def receive_timestamp():
    timestamp_data = request.json
    print(timestamp_data)
    if 'timestamp' in timestamp_data:
        timestamp = timestamp_data['timestamp']
        log_timestamp_to_file(timestamp)
        print(f"Received timestamp: {timestamp}")
        return jsonify({"message": "Timestamp received and logged successfully"}), 200
    else:
        return jsonify({"error": "Invalid data format. Expected 'timestamp' key."}), 400

if __name__ == '__main__':
    app.run(port=8765)
