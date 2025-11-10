#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>

struct Appointment {
    std::string client;
    std::string service;
    std::string staff;
    std::string date;
    std::string time;
};

class AppointmentSystem {
private:
    std::vector<Appointment> appointments;
    const std::string fileName = "appointments.txt";

    // --- Utility: trim and lowercase helpers ---
    static std::string trim(const std::string& s) {
        auto start = s.find_first_not_of(" \t\r\n");
        auto end = s.find_last_not_of(" \t\r\n");
        return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
    }

    static std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    // --- Load appointments from file (creates file if missing) ---
    void loadAppointments() {
        appointments.clear();

        // Create the file if it doesn't exist
        if (!std::filesystem::exists(fileName)) {
            std::ofstream newFile(fileName);
            newFile.close();
            return;
        }

        std::ifstream file(fileName);
        if (!file.is_open()) return;

        Appointment app;
        while (file >> std::quoted(app.client)
                    >> std::quoted(app.service)
                    >> std::quoted(app.staff)
                    >> std::quoted(app.date)
                    >> std::quoted(app.time)) {
            appointments.push_back(app);
        }
    }

    // --- Save appointments back to file ---
    void saveAppointments() {
        std::ofstream file(fileName, std::ios::trunc);
        for (const auto& app : appointments) {
            file << std::quoted(app.client) << " "
                 << std::quoted(app.service) << " "
                 << std::quoted(app.staff) << " "
                 << std::quoted(app.date) << " "
                 << std::quoted(app.time) << "\n";
        }
    }

public:
    AppointmentSystem() { loadAppointments(); }

    // --- Add appointment ---
    std::string addAppointment(const Appointment& a) {
        loadAppointments();

        // Check for duplicate/conflict
        for (const auto& app : appointments) {
            if (app.staff == a.staff && app.date == a.date && app.time == a.time) {
                return "⚠ Conflict: " + app.staff + " is already booked at this time.";
            }
        }

        appointments.push_back(a);
        saveAppointments();
        return "✅ Appointment added successfully!";
    }

    // --- Delete appointment ---
    std::string deleteAppointment(const std::string& client, const std::string& date, const std::string& time) {
        loadAppointments();
        std::string c = trim(client), d = trim(date), t = trim(time);
        auto before = appointments.size();

        appointments.erase(std::remove_if(appointments.begin(), appointments.end(),
            [&](const Appointment& a) {
                return trim(a.client) == c && trim(a.date) == d && trim(a.time) == t;
            }), appointments.end());

        if (appointments.size() == before)
            return "⚠ Appointment not found.";

        saveAppointments();
        return "🗑 Appointment deleted successfully.";
    }

    // --- List appointments (with optional search) ---
    std::string listAppointments(const std::string& search = "") {
        loadAppointments();
        std::stringstream ss;
        std::string term = toLower(trim(search));

        for (const auto& app : appointments) {
            std::string combined = toLower(app.client + " " + app.staff + " " + app.date + " " + app.service);
            if (!term.empty() && combined.find(term) == std::string::npos)
                continue;

            ss << "Client: " << app.client << "\n"
               << "Service: " << app.service << "\n"
               << "Staff: " << app.staff << "\n"
               << "Date: " << app.date << " | Time: " << app.time << "\n"
               << "--------------------------\n";
        }

        if (ss.str().empty())
            return "No appointments found.";
        return ss.str();
    }
};

// --- MAIN PROGRAM ---
int main(int argc, char* argv[]) {
    AppointmentSystem sys;

    if (argc < 2) {
        std::cout << "Usage:\n"
                  << "  add <client> <service> <staff> <date> <time>\n"
                  << "  list [search]\n"
                  << "  delete <client> <date> <time>\n";
        return 0;
    }

    std::string cmd = argv[1];

    if (cmd == "add" && argc == 7) {
        Appointment a = { argv[2], argv[3], argv[4], argv[5], argv[6] };
        std::cout << sys.addAppointment(a);
    }
    else if (cmd == "list") {
        std::string s = (argc == 3) ? argv[2] : "";
        std::cout << sys.listAppointments(s);
    }
    else if (cmd == "delete" && argc == 5) {
        std::cout << sys.deleteAppointment(argv[2], argv[3], argv[4]);
    }
    else {
        std::cout << "Invalid command or arguments.\n";
    }

    return 0;
}
