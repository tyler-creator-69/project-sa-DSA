import subprocess
import sys
import os
from PyQt6.QtWidgets import (
    QApplication, QWidget, QLabel, QLineEdit, QPushButton,
    QVBoxLayout, QHBoxLayout, QMessageBox, QComboBox, QGridLayout,
    QTableWidget, QTableWidgetItem, QHeaderView, QDateEdit, QTimeEdit
)
from PyQt6.QtCore import Qt, QDate, QTime


class AppointmentGUI(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Appointment System")
        self.setGeometry(200, 200, 950, 550)
        self.backend_path = os.path.join(os.getcwd(), "ahhhhh.exe")
        self.initUI()

    def initUI(self):
        layout = QVBoxLayout()

        # Title
        title = QLabel("📅 Appointment Scheduler")
        title.setAlignment(Qt.AlignmentFlag.AlignCenter)
        title.setStyleSheet("font-size: 22px; font-weight: bold; margin-bottom: 10px;")
        layout.addWidget(title)

        # --- Form section ---
        form = QGridLayout()
        self.client_input = QLineEdit()
        self.service_input = QComboBox()
        self.service_input.addItems(["Consultation", "Therapy", "Checkup", "Treatment"])
        self.staff_input = QComboBox()
        self.staff_input.addItems(["Dr. Smith", "Dr. Lee", "Dr. Perez", "Dr. Gomez"])
        self.date_input = QDateEdit(QDate.currentDate())
        self.date_input.setCalendarPopup(True)
        self.time_input = QTimeEdit(QTime.currentTime())

        form.addWidget(QLabel("Client Name:"), 0, 0)
        form.addWidget(self.client_input, 0, 1)
        form.addWidget(QLabel("Service Type:"), 1, 0)
        form.addWidget(self.service_input, 1, 1)
        form.addWidget(QLabel("Staff:"), 2, 0)
        form.addWidget(self.staff_input, 2, 1)
        form.addWidget(QLabel("Date:"), 3, 0)
        form.addWidget(self.date_input, 3, 1)
        form.addWidget(QLabel("Time:"), 4, 0)
        form.addWidget(self.time_input, 4, 1)
        layout.addLayout(form)

        # --- Buttons ---
        btn_layout = QHBoxLayout()
        self.add_btn = QPushButton("➕ Add")
        self.refresh_btn = QPushButton("🔄 Refresh")
        self.delete_btn = QPushButton("🗑 Delete")
        btn_layout.addWidget(self.add_btn)
        btn_layout.addWidget(self.refresh_btn)
        btn_layout.addWidget(self.delete_btn)
        layout.addLayout(btn_layout)

        # --- Search ---
        search_layout = QHBoxLayout()
        search_layout.addWidget(QLabel("🔍 Search:"))
        self.search_input = QLineEdit()
        self.search_input.setPlaceholderText("Type a client/staff/date and press Enter...")
        search_layout.addWidget(self.search_input)
        layout.addLayout(search_layout)

        # --- Table ---
        self.table = QTableWidget()
        self.table.setColumnCount(5)
        self.table.setHorizontalHeaderLabels(["Client", "Service", "Staff", "Date", "Time"])
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)
        layout.addWidget(self.table)

        # --- Connections ---
        self.add_btn.clicked.connect(self.add_appointment)
        self.refresh_btn.clicked.connect(lambda: self.search_backend(self.search_input.text()))
        self.delete_btn.clicked.connect(self.delete_selected)
        self.search_input.returnPressed.connect(lambda: self.search_backend(self.search_input.text().strip()))

        self.setLayout(layout)
        self.search_backend("")  # Load all on start

    # ---------------- BACKEND HANDLER ----------------
    def run_backend(self, args):
        """Run C++ backend command and return its output."""
        if not os.path.exists(self.backend_path):
            QMessageBox.critical(self, "Error", "C++ backend not found!")
            return ""
        try:
            result = subprocess.run(
                [self.backend_path] + args,
                capture_output=True,
                text=True,
                cwd=os.path.dirname(self.backend_path)  # ✅ ensures file access
            )
            return (result.stdout + result.stderr).strip()
        except Exception as e:
            QMessageBox.critical(self, "Error", str(e))
            return ""

    # ---------------- ADD FUNCTION ----------------
    def add_appointment(self):
        client = self.client_input.text().strip()
        if not client:
            QMessageBox.warning(self, "Error", "Client name required.")
            return

        service = self.service_input.currentText()
        staff = self.staff_input.currentText()
        date = self.date_input.date().toString("yyyy-MM-dd")
        time = self.time_input.time().toString("HH:mm")

        output = self.run_backend(["add", client, service, staff, date, time])
        QMessageBox.information(self, "Result", output)
        self.search_backend(self.search_input.text())
        self.clear_inputs()

    # ---------------- DELETE FUNCTION ----------------
    def delete_selected(self):
        row = self.table.currentRow()
        if row < 0:
            QMessageBox.warning(self, "Error", "Select a row to delete.")
            return

        client = self.table.item(row, 0).text()
        date = self.table.item(row, 3).text()
        time = self.table.item(row, 4).text()

        confirm = QMessageBox.question(
            self,
            "Confirm Delete",
            f"Delete appointment for {client} on {date} at {time}?",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No
        )

        if confirm == QMessageBox.StandardButton.Yes:
            output = self.run_backend(["delete", client, date, time])
            QMessageBox.information(self, "Result", output)
            self.search_backend(self.search_input.text())

    # ---------------- SEARCH FUNCTION ----------------
    def search_backend(self, text):
        args = ["list"]
        if text:
            args.append(text)
        output = self.run_backend(args)
        print("BACKEND OUTPUT:\n", output)  # For debugging
        self.populate_table(output)

    # ---------------- TABLE UPDATE ----------------
    def populate_table(self, output):
        self.table.setRowCount(0)
        if not output or "No appointments found" in output:
            return

        for entry in output.split("--------------------------"):
            lines = [l.strip() for l in entry.splitlines() if l.strip()]
            if len(lines) < 4:
                continue

            client = lines[0].split(": ", 1)[1]
            service = lines[1].split(": ", 1)[1]
            staff = lines[2].split(": ", 1)[1]
            date_time = lines[3].replace("Date: ", "")
            if "| Time:" in date_time:
                date, time = date_time.split("| Time:")
            else:
                date, time = date_time, ""
            row = self.table.rowCount()
            self.table.insertRow(row)
            for i, val in enumerate([client, service, staff, date.strip(), time.strip()]):
                self.table.setItem(row, i, QTableWidgetItem(val))

    # ---------------- UTILITIES ----------------
    def clear_inputs(self):
        self.client_input.clear()
        self.date_input.setDate(QDate.currentDate())
        self.time_input.setTime(QTime.currentTime())


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = AppointmentGUI()
    window.show()
    sys.exit(app.exec())
