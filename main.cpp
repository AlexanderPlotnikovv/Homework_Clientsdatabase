#include <iostream>
#include <vector>
#include <stdexcept>
#include <pqxx/pqxx>
#include <Windows.h>
#pragma execution_character_set( "utf-8" )

pqxx::connection conn("host=localhost port=5432 dbname=ClientsData user=postgres password=Alexander2023future!");

void createtable()
{
	pqxx::work tx{ conn };
	tx.exec( "CREATE TABLE IF NOT EXISTS clients("
		"ID serial PRIMARY KEY,"
		"name text,"
		"surname text,"
		"email text"
		");"
	);
	tx.exec( "CREATE TABLE IF NOT EXISTS phones("
		"client_id integer references clients(id),"
		"phone_count integer,"
		"phone_num text"
		");"
	);
	tx.commit();
}

void clientadd()
{
	std::string name;
	std::string surname;
	std::string email;
	std::cout << "Enter name, surname and email through space: ";
	std::cin >> name >> surname >> email;
	pqxx::work tx{ conn };
	tx.exec("INSERT into clients(name, surname, email) VALUES ('"+tx.esc(name)+"','" + tx.esc(surname) + "','" + tx.esc(email) + "')");
	tx.commit();
}

void phonenumadd()
{
	int id;
	std::cout << "Enter id of client: ";
	std::cin >> id;
	std::string phonenum;
	std::cout << "Enter client's phone number: ";
	std::cin >> phonenum;
	pqxx::work tx { conn };
	try
	{
		if ((tx.exec("SELECT phone_count FROM phones WHERE client_id = '" + tx.esc(std::to_string(id)) + "';")).empty())
		{
			tx.exec("INSERT into phones(client_id,phone_count,phone_num) VALUES ('" + tx.esc(std::to_string(id)) + "',1,'" + tx.esc(phonenum) + "');");
			tx.commit();
		}
		else
		{
			int res = tx.query_value<int>("SELECT MAX(phone_count) FROM phones WHERE client_id = '" + std::to_string(id) + "';");
			tx.exec("INSERT into phones(client_id,phone_count,phone_num) VALUES ('" + tx.esc(std::to_string(id)) + "','" + tx.esc(std::to_string(res + 1)) + "','" + tx.esc(phonenum) + "');");
			tx.commit();
		}
	}
	catch (const std::exception& ex)
	{
		std::cout << "Incorrect id to add phone!" << std::endl;
	}
}

void changedata()
{
	int id;
	std::cout << "Enter id of client: ";
	std::cin >> id;
	std::string a;
	std::cout << "What you want to change? (enter something from this variations: name, surname, email, phonenum)" << std::endl;
	std::cin >> a;
	try
	{
		if (a == "name")
		{
			std::string name;
			std::cout << "Enter new name: ";
			std::cin >> name;
			pqxx::work tx{ conn };
			tx.exec("UPDATE clients SET name = '" + tx.esc(name) + "' WHERE id = '" + tx.esc(std::to_string(id)) + "'");
			tx.commit();
		}
		else if (a == "surname")
		{
			std::string surname;
			std::cout << "Enter new surname: ";
			std::cin >> surname;
			pqxx::work tx{ conn };
			tx.exec("UPDATE clients SET surname = '" + tx.esc(surname) + "' WHERE id = '" + tx.esc(std::to_string(id)) + "'");
			tx.commit();
		}
		else if (a == "email")
		{
			std::string email;
			std::cout << "Enter new email: ";
			std::cin >> email;
			pqxx::work tx{ conn };
			tx.exec("UPDATE clients SET email = '" + tx.esc(email) + "' WHERE id = '" + tx.esc(std::to_string(id)) + "'");
			tx.commit();
		}
		else if (a == "phonenum")
		{
			std::string phonenum;
			std::cout << "Enter new phonenum: ";
			std::cin >> phonenum;
			pqxx::work tx{ conn };
			int res = tx.query_value<int>("SELECT MAX(phone_count) FROM phones WHERE client_id = '" + std::to_string(id) + "';");
			tx.exec("UPDATE phones SET phone_num = '" + tx.esc(phonenum) + "' WHERE client_id = '" + tx.esc(std::to_string(id)) + "' AND phone_count = '" + std::to_string(res) + "'");
			tx.commit();
		}
		else
		{
			std::cout << "Incorrect input!" << std::endl;
			changedata();
		}
	}
	catch (const std::exception& ex)
	{
		std::cout << "Incorrect id to change name!" << std::endl;
	}
}

void phonenumdelete()
{
	int id;
	std::cout << "Enter id of client whose phone number are you want to delete: ";
	std::cin >> id;
	pqxx::work tx{ conn };
	try
	{
		int res = tx.query_value<int>("SELECT MAX(phone_count) FROM phones WHERE client_id = '" + tx.esc(std::to_string(id)) + "';");
		tx.exec("DELETE FROM phones WHERE client_id = '" + std::to_string(id) + "' AND phone_count = '" + tx.esc(std::to_string(res)) + "';");
		tx.commit();
	}
	catch (const std::exception& ex)
	{
		std::cout << "Incorrcet id to delete phone or client hasn't already phone numbers!" << std::endl;
	}
}

void phonenumdeletecopy(int id)
{
	try
	{
		pqxx::work tx{ conn };
		int res = tx.query_value<int>("SELECT MAX(phone_count) FROM phones WHERE client_id = '" + tx.esc(std::to_string(id)) + "';");
		tx.exec("DELETE FROM phones WHERE client_id = '" + std::to_string(id) + "' AND phone_count = '" + tx.esc(std::to_string(res)) + "';");
		tx.commit();
	}
	catch (const std::exception& ex)
	{
		std::cout << "Incorrect id to delete phone or client hasn't already phone numbers!" << std::endl;
	}
}

void clientdelete()
{
	try
	{
		int id;
		std::cout << "Enter id of client are you want to delete: ";
		std::cin >> id;
		pqxx::work tx{ conn };
		while (!(tx.exec("SELECT phone_count FROM phones WHERE client_id = '" + tx.esc(std::to_string(id)) + "';")).empty())
		{
			phonenumdeletecopy(id);
		}
		tx.exec("DELETE FROM clients WHERE id = '" + tx.esc(std::to_string(id)) + "';");
		tx.commit();
	}
	catch (const std::exception& ex)
	{
		std::cout << "Incorrect client's id!" << std::endl;
	}
}

void clientfind()
{
	std::string a;
	std::cout << "How are you want to find client (id, name, surname, phone number (phonenum), email)?" << std::endl;
	std::cin >> a;
	if (a == "id")
	{
		try
		{
			int id;
			std::cout << "Enter id: ";
			std::cin >> id;
			pqxx::work tx{ conn };
			for (auto [name, surname, email] : tx.query<std::string, std::string, std::string>(
				"SELECT name , surname, email FROM clients WHERE id = '" + tx.esc(std::to_string(id)) + "';"))
			{
				std::cout << "Client: " << std::endl;
				std::cout << "Name: " << name << " Surname: " << surname << " Email: " << email << std::endl;
			}
			std::cout << "Phone number(s): ";
			for (auto [phone_num] : tx.query<std::string>(
				"SELECT phone_num FROM phones WHERE client_id = '" + tx.esc(std::to_string(id)) + "';"))
			{
				std::cout << phone_num << " ";
			}
			std::cout << std::endl;
		}
		catch (const std::exception& ex)
		{
			std::cout << "Incorrect id!" << std::endl;
		}
	}
	else if (a == "name")
	{
		try
		{
			int idhelp;
			std::string name;
			std::cout << "Enter name: ";
			std::cin >> name;
			pqxx::work tx{ conn };
			for (auto [id, surname, email] : tx.query<int, std::string, std::string>(
				"SELECT id, surname, email FROM clients WHERE name = '" + tx.esc(name) + "';"))
			{
				idhelp = id;
				std::cout << "Client: " << std::endl;
				std::cout << "Id: " << id << " Surname: " << surname << " Email: " << email << std::endl;
			}
			std::cout << "Phone number(s): ";
			for (auto [phone_num] : tx.query<std::string>(
				"SELECT phone_num FROM phones WHERE client_id = '" + tx.esc(std::to_string(idhelp)) + "';"))
			{
				std::cout << phone_num << " ";
			}
			std::cout << std::endl;
		}
		catch (const std::exception& ex)
		{
			std::cout << "Incorrect name!" << std::endl;
		}
	}
	else if (a == "surname")
	{
		try
		{
			int idhelp;
			std::string surname;
			std::cout << "Enter surname: ";
			std::cin >> surname;
			pqxx::work tx{ conn };
			for (auto [id, name, email, phonenum] : tx.query<int, std::string, std::string, std::string>(
				"SELECT id, name, email FROM clients WHERE surname = '" + tx.esc(surname) + "';"))
			{
				idhelp = id;
				std::cout << "Client: " << std::endl;
				std::cout << "Id: " << id << " Name: " << name << " Email: " << email << std::endl;
			}
			std::cout << "Phone number(s): ";
			for (auto [phone_num] : tx.query<std::string>(
				"SELECT phone_num FROM phones WHERE client_id = '" + tx.esc(std::to_string(idhelp)) + "';"))
			{
				std::cout << phone_num << " ";
			}
			std::cout << std::endl;
		}
		catch (const std::exception& ex)
		{
			std::cout << "Incorrect surname!" << std::endl;
		}
	}
	else if (a == "email")
	{
		try
		{
			int idhelp;
			std::string email;
			std::cout << "Enter email: ";
			std::cin >> email;
			pqxx::work tx{ conn };
			for (auto [id, name, surname] : tx.query<int, std::string, std::string>(
				"SELECT id, name, surname FROM clients WHERE email = '" + tx.esc(email) + "' "))
			{
				idhelp = id;
				std::cout << "Client: " << std::endl;
				std::cout << "Id: " << id << " Name: " << name << " Surname: " << tx.esc(surname) << std::endl;
			}
			std::cout << "Phone number(s): ";
			for (auto [phone_num] : tx.query<std::string>(
				"SELECT phone_num FROM phones WHERE client_id = '" + tx.esc(std::to_string(idhelp)) + "';"))
			{
				std::cout << phone_num << " ";
			}
			std::cout << std::endl;
		}
		catch (const std::exception& ex)
		{
			std::cout << "Incorrect email!" << std::endl;
		}
	}
	else if (a == "phonenum")
	{
		try
		{
			int idhelp;
			std::string phonenum;
			std::cout << "Enter phone number: ";
			std::cin >> phonenum;
			pqxx::work tx{ conn };
			for (auto [id, name, surname, email] : tx.query<int, std::string, std::string, std::string>(
				"select id,name,surname, email from clients inner join phones on clients.id = phones.client_id where phone_num = '" + tx.esc(phonenum) + "'"))
			{
				idhelp = id;
				std::cout << "Client: " << std::endl;
				std::cout << "Id: " << id << " Name: " << name << " Surname: " << surname << " Email: " << email << std::endl;
			}
			int phone_count = tx.query_value<int>("SELECT phone_count FROM phones WHERE phone_num = '" + tx.esc(phonenum) + "';");
			std::cout << "Phone number(s): ";
			for (auto [phone_num] : tx.query<std::string>(
				"SELECT phone_num FROM phones WHERE client_id = '" + tx.esc(std::to_string(idhelp)) + "';"))
			{
				if (tx.query_value<int>("SELECT phone_count FROM phones WHERE phone_num = '" + tx.esc(phone_num) + "';") != phone_count)
				{
					std::cout << phone_num << " ";
				}
			}
			std::cout << std::endl;
		}
		catch (const std::exception& ex)
		{
			std::cout << "Incorrect phone number!" << std::endl;
		}
	}
	else
	{
		std::cout << "Incorrect input!" << std::endl;
		clientfind();
	}
}

int main()
{
    setlocale(LC_ALL, "Russian");
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);
	pqxx::connection conn("host=localhost port=5432 dbname=ClientsData user=postgres password=Alexander2023future!");
	pqxx::work tx { conn };
	std::cout << "Hello, Dear friend! What action do you want to do?" << std::endl;
	int a = 1;
	while (a != 0)
	{
		std::cout << "1. Create table" << std::endl;
		std::cout << "2. Add new client" << std::endl;
		std::cout << "3. Add the phone number for exist client" << std::endl;
		std::cout << "4. Change data about client" << std::endl;
		std::cout << "5. Delete phone number at exist client" << std::endl;
		std::cout << "6. Delete client" << std::endl;
		std::cout << "7. Find client by data (name, surname, email, phone number)" << std::endl;
		std::cin >> a;
		switch (a)
		{
		case 1:
			createtable();
			break;
		case 2:
			clientadd();
			break;
		case 3:
			phonenumadd();
			break;
		case 4:
			changedata();
			break;
		case 5:
			phonenumdelete();
			break;
		case 6:
			clientdelete();
			break;
		case 7:
			clientfind();
			break;
		default:
			break;
		}
		if (a != 0)
		{
			std::cout << "What else? Enter 0 if you want to finish run queries" << std::endl;
		}
	}
	return 0;
}