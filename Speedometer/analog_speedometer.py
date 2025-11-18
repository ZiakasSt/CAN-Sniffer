# YES, OF COURSE THIS IS GPT-GENERATED

import sys, math, serial
from PyQt6.QtWidgets import QApplication, QWidget
from PyQt6.QtGui import QPainter, QPen, QColor, QFont
from PyQt6.QtCore import Qt, QTimer

SERIAL_PORT = "COM7"
BAUD_RATE = 921600

class SpeedometerUART(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Analog Speedometer")
        self.setGeometry(100, 100, 400, 400)
        self.speed, self.max_speed, self.redline_speed = 0, 200, 175

        try:
            self.ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
        except serial.SerialException:
            print(f"Cannot open serial port {SERIAL_PORT}")
            sys.exit(1)

        QTimer(self, timeout=self.read_serial).start(16)

    def read_serial(self):
        if self.ser.in_waiting:
            line = self.ser.readline().decode(errors='ignore').strip()
            try:
                # Look for the 'Data:' part
                if "Data:" in line:
                    data_str = line.split("Data:")[1].strip()  # "38 0B 03 87 EB 49 09 FF"
                    bytes_list = data_str.split()               # ['38', '0B', '03', ...]
                    if len(bytes_list) >= 2:
                        byte1 = int(bytes_list[1], 16)         # second byte in decimal
                        # Clamp to 0-max_speed just in case
                        self.speed = max(0, min(byte1, self.max_speed))
                        self.update()
            except (ValueError, IndexError):
                pass

    def paintEvent(self, event):
        p = QPainter(self)
        p.setRenderHint(QPainter.RenderHint.Antialiasing)
        
        # Fill background black
        p.fillRect(self.rect(), QColor(0, 0, 0))
        
        cx, cy = self.width()/2, self.height()/2
        r = min(cx, cy) * 0.8

        # Background circle (optional: darker gray instead of white)
        p.setBrush(QColor(30, 30, 30))  # dark gray
        p.setPen(Qt.PenStyle.NoPen)
        p.drawEllipse(int(cx-r), int(cy-r), int(2*r), int(2*r))

        # Tick marks and labels
        font = QFont("Arial", 10)
        p.setFont(font)
        for i in range(0, 9):
            angle = math.radians(225 - i * 270 / 8)
            x1, y1 = cx + (r - 15) * math.cos(angle), cy - (r - 15) * math.sin(angle)
            x2, y2 = cx + r * math.cos(angle), cy - r * math.sin(angle)
            p.setPen(QPen(Qt.GlobalColor.white, 2))  # ticks white
            p.drawLine(int(x1), int(y1), int(x2), int(y2))

            lx = cx + (r - 35) * math.cos(angle) - 10
            ly = cy - (r - 35) * math.sin(angle) + 5
            p.drawText(int(lx), int(ly), str(int(self.max_speed / 8 * i)))

        # Redline / warning zone
        if self.redline_speed < self.max_speed:
            sa = int((225 - (self.redline_speed / self.max_speed) * 270) * 16)
            span = int(-(270 - (self.redline_speed / self.max_speed) * 270) * 16)
            p.setPen(QPen(QColor(200, 0, 0), 6))
            p.drawArc(int(cx - r), int(cy - r), int(2*r), int(2*r), sa, span)

        # Needle
        angle = math.radians(225 - (self.speed / self.max_speed) * 270)
        nx, ny = cx + (r-40) * math.cos(angle), cy - (r-40) * math.sin(angle)
        p.setPen(QPen(QColor(200, 0, 0), 4))
        p.drawLine(int(cx), int(cy), int(nx), int(ny))

        # Center cap
        p.setBrush(QColor(255, 255, 255))  # white center for contrast
        p.setPen(Qt.PenStyle.NoPen)
        p.drawEllipse(int(cx-5), int(cy-5), 10, 10)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = SpeedometerUART()
    window.show()
    sys.exit(app.exec())
