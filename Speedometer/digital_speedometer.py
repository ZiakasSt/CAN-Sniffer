# YES, OF COURSE THIS IS GPT-GENERATED

import sys
import serial
import threading
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QLCDNumber, QLabel
from PyQt5.QtCore import Qt, QTimer

class Speedometer(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Digital Speedometer")
        self.setGeometry(200, 200, 300, 200)

        # Layout
        layout = QVBoxLayout()
        self.label = QLabel("Speed (km/h):", self)
        self.label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.label)

        # Digital display
        self.lcd = QLCDNumber(self)
        self.lcd.setDigitCount(5)
        self.lcd.display(0)
        layout.addWidget(self.lcd)

        self.setLayout(layout)

        # Serial setup
        self.serial_port = serial.Serial("COM7", 921600, timeout=1)

        # Speed value
        self.speed = 0

        # Background thread to read serial
        self.running = True
        self.thread = threading.Thread(target=self.read_serial)
        self.thread.start()

        # Timer to update GUI
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_display)
        self.timer.start(100)

    def read_serial(self):
        while self.running:
            try:
                line = self.serial_port.readline().decode(errors='ignore').strip()
                if "Data:" in line:
                    data_str = line.split("Data:")[1].strip()   # "38 0B 03 87 EB 49 09 FF"
                    bytes_list = data_str.split()               # ['38', '0B', '03', ...]
                    if len(bytes_list) >= 2:
                        byte1 = int(bytes_list[1], 16)         # second byte in decimal
                        self.speed = max(0, byte1)             # optional clamp to 0+
            except Exception as e:
                print("Serial error:", e)


    def update_display(self):
        self.lcd.display(self.speed)

    def closeEvent(self, event):
        self.running = False
        if self.thread.is_alive():
            self.thread.join()
        self.serial_port.close()
        event.accept()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = Speedometer()
    window.show()
    sys.exit(app.exec_())
