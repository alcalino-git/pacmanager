#include "glibmm/main.h"
#include <gtkmm.h>
#include "./search.hpp"
#include "gtkmm/box.h"
#include "gtkmm/enums.h"
#include "gtkmm/label.h"
#include "gtkmm/spinner.h"
#include "package.hpp"
#include "database.hpp"


class DefaultWindow : public Gtk::Window{
	Gtk::Paned screen_split;
	PackageDisplay package_display;
	SearchComponent search_component;
	PackageDatabase* database;
	Gtk::Spinner loading_spinner;

	public:
	DefaultWindow() {
		set_title("Pacmanager");
		set_size_request(1200, 800);

		auto loading_box = Gtk::Box();
		loading_box.set_orientation(Gtk::Orientation::VERTICAL);
		loading_box.set_halign(Gtk::Align::CENTER);
		loading_box.set_valign(Gtk::Align::CENTER);
		loading_box.set_hexpand(true);
		loading_box.set_vexpand(true);

		auto loading_text = Gtk::Label("Loading package database, please wait (I promise it's not hanged!)");
		loading_box.set_halign(Gtk::Align::CENTER);

		this->loading_spinner.set_spinning(true);
		this->loading_spinner.set_size_request(200,200);
		this->loading_spinner.set_expand(true);

		loading_box.append(loading_spinner);
		loading_box.append(loading_text);

		this->set_child(loading_box);


		this->search_component.signal_changed().connect([this](auto p) {
			this->package_display.set_package(p);
		});

		this->package_display.signal_update().connect([this](Package p) {
			std::cout << "Updates package " << p.get_property("Name") << "\n";
			this->database->update_package_state(p);
		});

		//Use a Gtk::Paned to render both the search component and PackageDisplay component
		this->screen_split.set_start_child(search_component);
		this->screen_split.set_end_child(package_display);
		this->screen_split.set_shrink_end_child(false);
		this->screen_split.set_resize_end_child(false);
		this->screen_split.set_shrink_start_child(false);
		this->screen_split.set_resize_start_child(true);
		this->screen_split.set_hexpand(true);

		Glib::signal_idle().connect_once([this](){
			this->database = new PackageDatabase();
			this->search_component.connect_database(this->database);
			this->set_child(screen_split);
		});


	}
};

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create("org.gtkmm.pacmanager");

	return app->make_window_and_run<DefaultWindow>(argc, argv);
}
