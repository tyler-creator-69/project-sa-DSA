#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

#ifdef _WIN32
#define RESET   ""
#define RED     ""
#define GREEN   ""
#define YELLOW  ""
#define CYAN    ""
#else
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#endif

struct Appointment {
    std::string name;
    std::string date;   // YYYY-MM-DD
    std::string time;   // HH:MM
    std::string category;

    std::string serialize() const {
        return name + ";" + date + ";" + time + ";" + category;
    }

    static Appointment deserialize(const std::string &line) {
        std::stringstream ss(line);
        Appointment a;
        getline(ss, a.name, ';');
        getline(ss, a.date, ';');
        getline(ss, a.time, ';');
        getline(ss, a.category, ';');
        return a;
    }
};

class AppointmentSystem {
private:
    std::vector<Appointment> appointments;

    static bool compareAppointments(const Appointment &a, const Appointment &b) {
        return std::tie(a.date, a.time) < std::tie(b.date, b.time);
    }

public:
    AppointmentSystem() {
        loadFromFile();
    }

    void addAppointment() {
        Appointment a;

        std::cout << CYAN << "\nEnter name: " << RESET;
        std::cin.ignore();
        getline(std::cin, a.name);

        std::cout << CYAN << "Enter date (YYYY-MM-DD): " << RESET;
        std::cin >> a.date;

        std::cout << CYAN << "Enter time (HH:MM): " << RESET;
        std::cin >> a.time;

        std::cout << CYAN << "Choose category (Work / Health / Personal): " << RESET;
        std::cin >> a.category;

        appointments.push_back(a);
        sortAppointments();

        std::cout << GREEN << "Appointment added successfully!\n" << RESET;
    }

    void editAppointment() {
        viewAppointments();
        if (appointments.empty()) return;

        int id;
        std::cout << YELLOW << "\nEnter appointment number to edit: " << RESET;
        std::cin >> id;

        if (id < 1 || id > appointments.size()) {
            std::cout << RED << "Invalid number.\n" << RESET;
            return;
        }

        Appointment &a = appointments[id - 1];

        std::cin.ignore();
        std::string input;

        std::cout << CYAN << "Enter new name (" << a.name << "): " << RESET;
        getline(std::cin, input);
        if (!input.empty()) a.name = input;

        std::cout << CYAN << "Enter new date (" << a.date << "): " << RESET;
        getline(std::cin, input);
        if (!input.empty()) a.date = input;

        std::cout << CYAN << "Enter new time (" << a.time << "): " << RESET;
        getline(std::cin, input);
        if (!input.empty()) a.time = input;

        std::cout << CYAN << "Enter new category (" << a.category << "): " << RESET;
        getline(std::cin, input);
        if (!input.empty()) a.category = input;

        sortAppointments();
        std::cout << GREEN << "Appointment updated!\n" << RESET;
    }

    void deleteAppointment() {
        viewAppointments();
        if (appointments.empty()) return;

        int id;
        std::cout << RED << "\nEnter appointment number to delete: " << RESET;
        std::cin >> id;

        if (id < 1 || id > appointments.size()) {
            std::cout << RED << "Invalid number.\n" << RESET;
            return;
        }

        appointments.erase(appointments.begin() + id - 1);
        std::cout << GREEN << "Deleted.\n" << RESET;
    }

    void viewAppointments() {
        if (appointments.empty()) {
            std::cout << RED << "\nNo appointments.\n" << RESET;
            return;
        }

        std::cout << YELLOW << "\n===== Appointments =====\n" << RESET;
        int i = 1;
        for (const auto &a : appointments) {

            std::string color = CYAN;
            if (a.category == "Work") color = GREEN;
            else if (a.category == "Health") color = RED;
            else if (a.category == "Personal") color = YELLOW;

            std::cout << color
                      << i++ << ". [" << a.category << "] "
                      << a.date << " " << a.time
                      << " - " << a.name
                      << RESET << "\n";
        }
    }

    void searchAppointment() {
        std::cin.ignore();
        std::string term;

        std::cout << CYAN << "\nEnter search term: " << RESET;
        getline(std::cin, term);

        bool found = false;
        for (const auto &a : appointments) {
            if (a.name.find(term) != std::string::npos ||
                a.category.find(term) != std::string::npos) {
                std::cout << GREEN << a.date << " " << a.time << " - "
                          << a.name << " (" << a.category << ")\n" << RESET;
                found = true;
            }
        }

        if (!found)
            std::cout << RED << "No matching appointments.\n" << RESET;
    }

    void showUpcoming() {
        std::cout << CYAN << "\nUpcoming Appointments (Next 7 days):\n" << RESET;

        auto now = std::chrono::system_clock::now();
        auto now_time = std::chrono::system_clock::to_time_t(now);

        bool any = false;

        for (const auto &a : appointments) {

            // -----------------------------
            // Manual date/time parsing
            // -----------------------------
            std::tm appt_tm = {};
            char dash1, dash2, space, colon;

            std::stringstream ss(a.date + " " + a.time);

            ss >> appt_tm.tm_year >> dash1 >> appt_tm.tm_mon >> dash2 >> appt_tm.tm_mday;
            ss >> appt_tm.tm_hour >> colon >> appt_tm.tm_min;

            // Adjust for struct tm format
            appt_tm.tm_year -= 1900;    // Years since 1900
            appt_tm.tm_mon -= 1;        // Months 0-11

            // Convert to time_t
            std::time_t appt_time_t = std::mktime(&appt_tm);
            auto appt_time = std::chrono::system_clock::from_time_t(appt_time_t);

            // Difference in hours
            auto diff = std::chrono::duration_cast<std::chrono::hours>(appt_time - now).count();

            // -----------------------------
            // Check if within 7 days
            // -----------------------------
            if (diff >= 0 && diff <= 24 * 7) {
                std::cout << YELLOW << a.date << " " << a.time << " - "
                          << a.name << " (" << a.category << ")\n" << RESET;
                any = true;
            }
        }

        if (!any)
            std::cout << RED << "No upcoming appointments in the next 7 days.\n" << RESET;
    }


    void sortAppointments() {
        std::sort(appointments.begin(), appointments.end(), compareAppointments);
    }

    void saveToFile() {
        std::ofstream out("appointments.txt");
        for (const auto &a : appointments)
            out << a.serialize() << "\n";
    }

    void loadFromFile() {
        std::ifstream in("appointments.txt");
        if (!in) return;

        std::string line;
        while (getline(in, line)) {
            if (!line.empty())
                appointments.push_back(Appointment::deserialize(line));
        }

        sortAppointments();
    }
};

int main() {
    AppointmentSystem system;

    int option;
    do {
        std::cout << CYAN << "\n=== Advanced Appointment System ===\n" << RESET;
        std::cout << "1. Add Appointment\n";
        std::cout << "2. View Appointments\n";
        std::cout << "3. Delete Appointment\n";
        std::cout << "4. Edit Appointment\n";
        std::cout << "5. Search Appointment\n";
        std::cout << "6. Show Upcoming Appointments\n";
        std::cout << "7. Save\n";
        std::cout << "0. Exit\n";

        std::cout << YELLOW << "Choose option: " << RESET;
        std::cin >> option;

        switch (option) {
            case 1: system.addAppointment(); break;
            case 2: system.viewAppointments(); break;
            case 3: system.deleteAppointment(); break;
            case 4: system.editAppointment(); break;
            case 5: system.searchAppointment(); break;
            case 6: system.showUpcoming(); break;
            case 7: system.saveToFile(); break;
            case 0: system.saveToFile(); break;
            default: std::cout << RED << "Invalid option.\n" << RESET;
        }

    } while (option != 0);

    return 0;
}
