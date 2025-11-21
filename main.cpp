#include <iostream>
#include <string>
#include <vector>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// EJERCICIO: mostrar nombre de episodios en los que sale el personaje, seguir preguntando personajes hasta introducir cadena vacía

struct Character
{
  std::string name;
  std::string origin;
  std::string species;
  std::string status;
  std::vector<std::string> episodes;
};

std::vector<Character> parse_characters(const std::string &body)
{
  std::vector<Character> characters;

  json j = json::parse(body);

  if (!j.contains("results") || !j["results"].is_array())
  {
    return characters;
  }

  for (const auto &item : j["results"])
  {
    Character c;

    c.name = item.value("name", "Desconocido");
    c.status = item.value("status", "Desconocido");
    c.species = item.value("species", "Desconocida");

    if (item.contains("origin") && item["origin"].is_object())
    {
      c.origin = item["origin"].value("name", "Desconocido");
    }
    else
    {
      c.origin = "Desconocido";
    }

    if (item.contains("episode") && item["episode"].is_array())
    {
        for (const auto &url: item["episode"]) {
            c.episodes.push_back(url);
        }
    }

    characters.push_back(std::move(c));
  }

  return characters;
}

std::string parse_episode(const std::string &body) {
    json j = json::parse(body);
    return j["name"];
}

void print_vector(std::vector<std::string> const & v) {
    for (const auto &elem: v) {
        std::cout << elem << std::endl;
    }
    std::cout << std::endl;
}

int main()
{

    std::cout << "Introduce el nombre (o parte del nombre) del personaje de Rick & Morty: ";
    std::string search;

    while (true) {
        std::getline(std::cin, search);

        if (search.empty())
        {
            std::cerr << "Cadena de busqueda vacia. Terminando.\n";
            return 1;
        }

        // Petición GET con cpr, usando parámetros (hace URL encoding por ti)
        auto response = cpr::Get(
            cpr::Url{"https://rickandmortyapi.com/api/character/"},
            cpr::Parameters{{"name", search}});

        if (response.error)
        {
            std::cerr << "Error en la peticion HTTP: " << response.error.message << "\n";
            return 1;
        }

        if (response.status_code != 200)
        {
            std::cerr << "La API devolvio codigo HTTP "
                      << response.status_code << "\nCuerpo:\n"
                      << response.text << "\n";
            return 1;
        }

        auto characters = parse_characters(response.text);
        if (characters.empty())
        {
            std::cout << "No se encontraron resultados para: " << search << "\n";
            return 0;
        }

        // Mostrar listado de resultados
        std::cout << "\nResultados encontrados:\n";
        for (std::size_t i{0}; i < characters.size(); ++i)
        {
            std::cout << i << ") " << characters.at(i).name << '\n';
        }

        // Seleccionar uno
        std::cout << "\nSelecciona el indice del personaje que te interesa: ";
        std::size_t index = 0;
        if (!(std::cin >> index))
        {
            std::cerr << "Entrada no valida.\n";
            return 1;
        }

        // limpiar salto de línea que queda en el buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (index >= characters.size())
        {
            std::cerr << "Indice fuera de rango.\n";
            return 1;
        }

        auto &c = characters.at(index);

        int i = 0;
        for (const std::string &url: c.episodes) {

            auto res = cpr::Get(cpr::Url{url});

            if (res.error)
            {
                std::cerr << "Error en la peticion HTTP: " << res.error.message << "\n";
                return 1;
            }

            if (res.status_code != 200)
            {
                std::cerr << "La API devolvio codigo HTTP "
                          << res.status_code << "\nCuerpo:\n"
                          << res.text << "\n";
                return 1;
            }

            auto episode_name = parse_episode(res.text);

            c.episodes[i] = episode_name;
            i++;
        }


        std::cout << "\n--- Detalles del personaje ---\n";
        std::cout << "Nombre : " << c.name << '\n';
        std::cout << "Planeta (origen): " << c.origin << '\n';
        std::cout << "Especie: " << c.species << '\n';
        std::cout << "Status : " << c.status << '\n';
        if (!c.episodes.empty()) {
            std::cout << "Episodes: \n";
            print_vector(c.episodes);
        } else {
            std::cout << "No se han encontrado episodios de este personaje.\n";
        }

        std::cout << "\nIntroduce el nombre (o parte del nombre) de otro personaje de Rick & Morty: ";
    }
}
