#include <stdio.h>
#include <gtkmm.h>
#include "./search.hpp"
#include "package.hpp"
#include "database.hpp"


class DefaultWindow : public Gtk::Window{
	Gtk::Paned screen_split;
	PackageDisplay package_display;
	SearchComponent search_component;
	PackageDatabase* database;

	public:
	DefaultWindow() {
		set_title("Pacmanager");
		set_size_request(1200, 800);

		this->database = new PackageDatabase();
		this->search_component.connect_database(this->database);
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

		this->set_child(screen_split);
	}
};

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create("org.gtkmm.pacmanager");

	return app->make_window_and_run<DefaultWindow>(argc, argv);
}
