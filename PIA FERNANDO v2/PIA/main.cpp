#include<iostream>
#include<Windows.h>
#include"resource.h"
#include<fstream>
#include<string>
#include<CommCtrl.h>
#include<stdio.h>
#include<vector>
#include<algorithm>
#include <ctime>
#include <sstream>
#include <algorithm>

using namespace std;
/*
	LISTO	Usuarios: arbol binario
	LISTO	Pilotos: QuickSort
	LISTO	Vuelos: Heapsort
	Pasajeros: normal
	Boletos: normal
*/

//Altas, bajas y modificacion de todas las estructuras
static int id = 0;
static int pasId = 0;
static int boletoId = 0;
int h;

bool seisMeses(SYSTEMTIME fecha) {
	SYSTEMTIME hoy;
	GetLocalTime(&hoy);
	SYSTEMTIME seisMesesDespues = hoy;
	seisMesesDespues.wMonth += 6;
	if (seisMesesDespues.wMonth > 12) {
		seisMesesDespues.wYear += 1;
		seisMesesDespues.wMonth -= 12;
	}
	if (fecha.wYear > seisMesesDespues.wYear ||
		(fecha.wYear == seisMesesDespues.wYear && fecha.wMonth > seisMesesDespues.wMonth) ||
		(fecha.wYear == seisMesesDespues.wYear && fecha.wMonth == seisMesesDespues.wMonth && fecha.wDay >= seisMesesDespues.wDay)) {
		return true;
	}
	else {
		if (fecha.wYear == hoy.wYear && fecha.wMonth == hoy.wMonth && fecha.wDay == hoy.wDay) {
			return true;
			//std::cout << "La variable 'a' es mayor a 6 meses a partir de hoy y es igual a hoy" << std::endl;
		}
		else return false;
		//std::cout << "La variable 'a' no es mayor a 6 meses a partir de hoy." << std::endl;
	}
}

//Arbol binario
struct User
{
	char usuario[100];
	char correo[100];
	char contra[100];
	char nombre[100];
	char apellidoP[100];
	char apellidoM[100];
	char nacimiento[100];
	bool genero;
	char foto[100];
	char registro[100];
	char usuRegistro[100];
	User* izq = NULL, * der = NULL;
};	User* inicioU = NULL, * auxU = NULL, * pivoteU = new User(), * admin = new User(), * actual = new User();

#pragma region ArbolBinario
User* insertarUsuario(User* nodo, User* nuevo)
{
	if (nodo == NULL) {
		nodo = new User();
		nodo = nuevo;
	}
	else {
		if (strcmp(nuevo->usuario, nodo->usuario) < 0)
			nodo->izq = insertarUsuario(nodo->izq, nuevo);
		else
			nodo->der = insertarUsuario(nodo->der, nuevo);
	}
	return nodo;
}

void buscarUsuario(const char* usuario)
{
	// Buscar en el árbol
	auxU = inicioU;
	while (auxU != nullptr) {
		if (strcmp(usuario, auxU->usuario) == 0) {
			// Encontrado en el árbol
			return;
		}
		else if (strcmp(usuario, auxU->usuario) < 0) {
			auxU = auxU->izq;
		}
		else {
			auxU = auxU->der;
		}
	}

	// Si no se encontró en el árbol, buscar en el archivo
	ifstream archivoU("usuarios", ios::binary);
	if (!archivoU.is_open()) {
		cerr << "No se pudo abrir el archivo de usuarios." << endl;
		return;
	}

	User usuarioArchivo;
	while (archivoU.read(reinterpret_cast<char*>(&usuarioArchivo), sizeof(User))) {
		if (strcmp(usuario, usuarioArchivo.usuario) == 0) {
			// Encontrado en el archivo
			auxU = new User(usuarioArchivo); // Crear una copia para evitar problemas de memoria
			archivoU.close();
			return;
		}
	}

	archivoU.close();
	// Usuario no encontrado ni en el árbol ni en el archivo
	auxU = nullptr;
}
void modificarUsuario(User*& modif, User* pivote) {
	strcpy_s(modif->correo, pivote->correo);
	strcpy_s(modif->contra, pivote->contra);
	strcpy_s(modif->nombre, pivote->nombre);
	strcpy_s(modif->apellidoP, pivote->apellidoP);
	strcpy_s(modif->apellidoM, pivote->apellidoM);
	strcpy_s(modif->nacimiento, pivote->nacimiento);
	if (pivote->genero == 1)
		modif->genero = 1;
	else
		modif->genero = 0;
	strcpy_s(modif->foto, pivote->foto);
}

void eliminarUsuario(const char* usuario) {
	pivoteU = new User();
	pivoteU = auxU;
	auxU = inicioU;
	while (auxU != nullptr) {
		// iqzuierda
		if (strcmp(usuario, auxU->usuario) < 0) {
			pivoteU = auxU;
			auxU = auxU->izq;
		}
		// derecha
		else if (strcmp(usuario, auxU->usuario) > 0) {
			pivoteU = auxU;
			auxU = auxU->der;
		}
		// Lo encontro
		else {
			if (inicioU->der == NULL && inicioU->izq == NULL) {
				auxU = NULL;
				pivoteU = NULL;
				inicioU = NULL;
			}
			else {
				pivoteU->der = auxU->der;
				pivoteU->izq = auxU->izq;
				auxU = nullptr;
				delete auxU;
			}
		}
	}
	if (auxU == nullptr)
		cout << "No se encuentra";
}

void limpiarVentanaUsuario(HWND hwnd)
{
	SetDlgItemText(hwnd, USUARIO, 0);
	SetDlgItemText(hwnd, NOMBRE, 0);
	SetDlgItemText(hwnd, APELLIDOP, 0);
	SetDlgItemText(hwnd, APELLIDOM, 0);
	SetDlgItemText(hwnd, CORREO, 0);
	SetDlgItemText(hwnd, CONTRA, 0);
	SendDlgItemMessage(hwnd, RMUJER, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	SendDlgItemMessage(hwnd, RHOMBRE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
	HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, "", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	HWND hFoto = GetDlgItem(hwnd, FOTOUSUARIO);
	SendMessage(hFoto, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
}

void llenarUsuarios(HWND hwnd, User* nodo)
{
	ifstream archivoU("usuarios", ios::binary | ios::in);
	if (!archivoU.is_open()) {
		// Manejar el caso en el que el archivo no se pueda abrir
		return;
	}

	// Leer los datos de usuario desde el archivo y agregarlos a la lista
	User usuario;
	while (archivoU.read(reinterpret_cast<char*>(&usuario), sizeof(User)))
	{
		SendMessage(GetDlgItem(hwnd, LISTAUSUARIOS), LB_ADDSTRING, 0, (LPARAM)usuario.usuario);
	}

	archivoU.close();
}

void llenarUsuariosR(HWND hwnd, User* nodo)
{
	if (nodo == nullptr) return;

	llenarUsuariosR(hwnd, nodo->izq);
	SendMessage(GetDlgItem(hwnd, LISTAUSUARIOS), LB_ADDSTRING, 0, (LPARAM)nodo->usuario);
	llenarUsuariosR(hwnd, nodo->der);
}

SYSTEMTIME convertirFecha(const char* tiempo)
{
	SYSTEMTIME st;
	sscanf_s(tiempo, "%hd/%hd/%hd %hd:%hd:%hd", &st.wMonth, &st.wDay, &st.wYear, &st.wHour, &st.wMinute, &st.wSecond);
	return st;
}

#pragma endregion

void guardarUsuario(ofstream& archivo, const User* usuario)
{
	// Mover el puntero al final del archivo
	archivo.seekp(0, ios::end);

	// Escribir el usuario al final del archivo
	archivo.write(reinterpret_cast<const char*>(usuario), sizeof(User));
}

void guardarUsuariosRecursivo(ofstream& archivo, User* nodo)
{
	if (nodo == nullptr)
		return;

	// Guardar los datos del usuario actual en el archivo
	guardarUsuario(archivo, nodo);

	// Recorrer los subárboles izquierdo y derecho
	guardarUsuariosRecursivo(archivo, nodo->izq);
	guardarUsuariosRecursivo(archivo, nodo->der);
}

void guardarUsuarios()
{
	ofstream archivoU("usuarios", ios::binary | ios::out);
	if (!archivoU.is_open())
	{
		cerr << "No se pudo abrir el archivo para escritura." << endl;
		return;
	}

	guardarUsuariosRecursivo(archivoU, inicioU);

	archivoU.close();
}

void leerUsuarios()
{
	std::ifstream archivo("usuarios", std::ios::binary);

	if (!archivo) {
		std::cerr << "No se pudo abrir el archivo." << std::endl;
		return;
	}

	User usuario;
	while (archivo.read(reinterpret_cast<char*>(&usuario), sizeof(User)))
	{
		// Procesa los datos del usuario
		std::cout << "Usuario: " << usuario.usuario << std::endl;
		std::cout << "Correo: " << usuario.correo << std::endl;
		std::cout << "Contra: " << usuario.contra << std::endl;
		std::cout << "Nombre: " << usuario.nombre << std::endl;
		std::cout << "ApellidoP: " << usuario.apellidoP << std::endl;
		std::cout << "ApellidoM: " << usuario.apellidoM << std::endl;
		std::cout << "Nacimiento: " << usuario.nacimiento << std::endl;
		std::cout << "Genero: " << usuario.genero << std::endl;
		std::cout << "Foto: " << usuario.foto << std::endl;
		std::cout << "Registro: " << usuario.registro << std::endl;
		std::cout << "usuRegistro: " << usuario.usuRegistro << std::endl;
	}
	archivo.close();
}

bool nombreRepetidoU(const char* usuario, User* inicioU)
{
	User* temp = inicioU;
	while (temp != NULL)
	{
		if (strcmp(usuario, temp->usuario) == 0)
		{
			// Se encontró un piloto con el mismo nombre
			return true;
		}
		temp = temp->der;
	}
	// No se encontró ningún piloto con el mismo nombre
	return false;
}

bool esMayorDe18(const char* nacimiento)
{
	// Obtener la fecha actual
	std::time_t tiempoActual = std::time(nullptr);
	std::tm* tmActual = std::localtime(&tiempoActual);

	// Convertir la fecha de nacimiento del usuario en un objeto std::tm
	std::tm tmNacimiento = {};
	std::stringstream ss(nacimiento);
	ss >> tmNacimiento.tm_mon;
	ss.ignore();
	ss >> tmNacimiento.tm_mday;
	ss.ignore();
	ss >> tmNacimiento.tm_year;
	tmNacimiento.tm_year -= 1900; // tm_year cuenta desde 1900
	tmNacimiento.tm_mon -= 1; // tm_mon cuenta desde 0
	tmNacimiento.tm_hour = tmNacimiento.tm_min = tmNacimiento.tm_sec = 0; // Ajuste de la hora para la comparación

	// Calcular la diferencia de años
	int edad = tmActual->tm_year - tmNacimiento.tm_year;

	// Comprobar si el usuario tiene más de 18 años
	if (tmActual->tm_mon < tmNacimiento.tm_mon ||
		(tmActual->tm_mon == tmNacimiento.tm_mon && tmActual->tm_mday < tmNacimiento.tm_mday)) {
		edad--; // Restar 1 si todavía no es su cumpleaños
	}

	return edad >= 18;
}

//QuickSort
struct Piloto
{
	char nombre[100];
	char apellidoP[100];
	char apellidoM[100];
	int horas;
	bool status;	//1 disponible,0 ausente
	Piloto* sig = NULL, * ant = NULL;
};	Piloto* inicioPilo = NULL, * auxP = NULL, * pivoteP = NULL;

#pragma region QuickSort

void swap(Piloto*& a, Piloto*& b) {
	if (a->sig == b) {
		a->sig = b->sig;
		b->ant = a->ant;
		a->ant = b;
		b->sig = a;
	}
	else if (b->sig == a) {
		b->sig = a->sig;
		a->ant = b->ant;
		b->ant = a;
		a->sig = b;
	}
	else {
		Piloto* temp = a->sig;
		a->sig = b->sig;
		b->sig = temp;

		temp = a->ant;
		a->ant = b->ant;
		b->ant = temp;
	}

	if (a->sig != NULL) {
		a->sig->ant = a;
	}
	if (a->ant != NULL) {
		a->ant->sig = a;
	}
	if (b->sig != NULL) {
		b->sig->ant = b;
	}
	if (b->ant != NULL) {
		b->ant->sig = b;
	}
	Piloto* temp = a;
	a = b;
	b = temp;
}


Piloto* partition(Piloto* l, Piloto* h) {
	char* x = h->nombre;

	Piloto* i = l->ant;

	for (Piloto* j = l; j != h; j = j->sig) {
		if (strcmp(j->nombre, x) <= 0) {
			i = (i == NULL) ? l : i->sig;
			swap(i, j);
		}
	}
	i = (i == NULL) ? l : i->sig;
	swap(i, h);
	return i;
}

void quickSort(Piloto* l, Piloto* h) {
	if (h != NULL && l != h && l != h->sig) {
		Piloto* p = partition(l, h);
		quickSort(l, p->ant);
		quickSort(p->sig, h);
	}
}

void quickSort(Piloto** headRef) {
	Piloto* h = *headRef;
	while (h->sig != NULL) {
		h = h->sig;
	}

	quickSort(*headRef, h);
}

void agregarPiloto(Piloto** head, const char* nombre, const char* apellidoP, const char* apellidoM, int horas) {
	Piloto* nuevoPiloto = new Piloto;
	strcpy_s(nuevoPiloto->nombre, nombre);
	strcpy_s(nuevoPiloto->apellidoP, apellidoP);
	strcpy_s(nuevoPiloto->apellidoM, apellidoM);
	nuevoPiloto->horas = horas;
	nuevoPiloto->status = true;
	if (*head == NULL) {
		*head = nuevoPiloto;
	}
	else {
		Piloto* temp = *head;
		while (temp->sig != NULL) {
			temp = temp->sig;
		}
		temp->sig = nuevoPiloto;
		nuevoPiloto->ant = temp;
	}
}
void agregarPiloto(Piloto** head, const char* nombre, const char* apellidoP, const char* apellidoM, int horas, bool status) {
	Piloto* nuevoPiloto = new Piloto;
	strcpy_s(nuevoPiloto->nombre, nombre);
	strcpy_s(nuevoPiloto->apellidoP, apellidoP);
	strcpy_s(nuevoPiloto->apellidoM, apellidoM);
	nuevoPiloto->horas = horas;
	nuevoPiloto->status = status;
	if (*head == NULL) {
		*head = nuevoPiloto;
	}
	else {
		Piloto* temp = *head;
		while (temp->sig != NULL) {
			temp = temp->sig;
		}
		temp->sig = nuevoPiloto;
		nuevoPiloto->ant = temp;
	}
}

void eliminarPiloto(Piloto** head, const char* nombre) {
	Piloto* temp = *head;
	while (temp != NULL) {
		if (strcmp(temp->nombre, nombre) == 0) {
			if (temp == *head) {
				*head = (*head)->sig;
				if (*head != NULL) {
					(*head)->ant = NULL;
				}
			}
			else {
				if (temp->sig != NULL) {
					temp->sig->ant = temp->ant;
				}
				if (temp->ant != NULL) {
					temp->ant->sig = temp->sig;
				}
			}
			delete temp;
			return;
		}
		temp = temp->sig;
	}
}

void modificarPiloto(Piloto* head, const char* nombre, const char* nuevoNombre, const char* nuevoApellidoP, const char* nuevoApellidoM, int nuevasHoras) {
	Piloto* temp = head;
	while (temp != NULL) {
		if (strcmp(temp->nombre, nombre) == 0) {
			strcpy_s(temp->nombre, nuevoNombre);
			strcpy_s(temp->apellidoP, nuevoApellidoP);
			strcpy_s(temp->apellidoM, nuevoApellidoM);
			temp->horas = nuevasHoras;
			return;
		}
		temp = temp->sig;
	}
}

void mostrarPiloto(Piloto* head, const char* nombre) {
	Piloto* temp = head;
	while (temp != NULL) {
		if (strcmp(temp->nombre, nombre) == 0) {
			auxP = temp;
			return;
		}
		temp = temp->sig;
	}
}
#pragma endregion

void guardarPilotos(Piloto* inicioPilo) {
	std::ofstream file("pilotos.bin", std::ios::binary);
	Piloto* current = inicioPilo;
	while (current != NULL) {
		file.write((char*)current, sizeof(Piloto));
		current = current->sig;
	}
	file.close();
}

void cargarPilotos(Piloto** inicioPilo) {
	std::ifstream file("pilotos.bin", std::ios::binary);
	Piloto* current = new Piloto;
	while (file.read((char*)current, sizeof(Piloto))) {
		agregarPiloto(inicioPilo, current->nombre, current->apellidoP, current->apellidoM, current->horas, current->status);

		//current->sig = *inicioPilo;
		//if (*inicioPilo != NULL) {
		//	(*inicioPilo)->ant = current;
		//}
		//*inicioPilo = current;
		current = new Piloto;
	}
	delete current;
	file.close();
}

void guardarPiloto()
{
	ofstream archivoP;
	archivoP.open("pilotos", ios::binary | ios::out | ios::app);
	if (archivoP.is_open()) {
		Piloto* aux = inicioPilo;
		while (aux != nullptr)
		{
			archivoP.write(reinterpret_cast<char*>(aux), sizeof(Piloto));
			aux = aux->sig;
		}
	}
	archivoP.close();
}

void leerArcPiloto() {
	ifstream archivoP;
	archivoP.open("pilotos", ios::binary | ios::in);
	if (archivoP.is_open()) {
		while (!archivoP.eof())
		{
			Piloto* nuevo = new Piloto;
			archivoP.read(reinterpret_cast<char*>(nuevo), sizeof(Piloto));
			if (!archivoP.eof()) {
				nuevo->sig = nullptr;
				nuevo->ant = nullptr;
				// Verificar si el nombre del piloto ya existe en la lista
				bool existe = false;
				Piloto* temp = inicioPilo;
				while (temp != nullptr) {
					if (strcmp(temp->nombre, nuevo->nombre) == 0) {
						existe = true;
						break;
					}
					temp = temp->sig;
				}
				// Agregar el nuevo piloto solo si no existe en la lista
				if (!existe) {
					if (inicioPilo == nullptr) {
						inicioPilo = nuevo;
					}
					else {
						Piloto* aux = inicioPilo;
						while (aux->sig != nullptr) {
							aux = aux->sig;
						}
						aux->sig = nuevo;
						nuevo->ant = aux;
					}
				}
				else {
					// Si el piloto ya existe, liberar la memoria del nuevo nodo
					delete nuevo;
				}
			}
		}
	}
	archivoP.close();
}

bool nombreRepetidoP(const char* nombre, Piloto* inicioPilo)
{
	Piloto* temp = inicioPilo;
	while (temp != NULL) {
		if (strcmp(nombre, temp->nombre) == 0) {
			// Se encontró un piloto con el mismo nombre
			return true;
		}
		temp = temp->sig;
	}
	// No se encontró ningún piloto con el mismo nombre
	return false;
}

//HeapSort
struct Vuelos {
	char origen[100];
	char destino[100];
	char fecha[100];
	char registro[100];
	char usuRegistro[100];
	char piloto[100];
	int id;
	int tipoAvion;	//0 Ancho, 1 Estrecho
	int asientos;
	int totalAsientos;
	int totalChild;
	int totalOld;
	float childPercent;
	float oldPercent;
	int estatus; //0 registrado, 1 Abordado, 2 cancelado
	Vuelos* sig = NULL, * ant = NULL;
};	Vuelos* inicioV = NULL, * auxV = NULL, * pivoteV = NULL;

#pragma region HeapSort
void insertarVuelo(Vuelos* nuevo) {
	if (inicioV == NULL) {
		inicioV = nuevo;
	}
	else {
		Vuelos* temp = inicioV;
		while (temp->sig != NULL) {
			temp = temp->sig;
		}
		temp->sig = nuevo;
		nuevo->ant = temp;
	}
}
void cancelarVuelo(int id)
{
	Vuelos* temp = inicioV;
	while (temp != NULL) {
		if (temp->id == id) {
			temp->estatus = 2;
			break;
		}
		temp = temp->sig;
	}
}

void vueloAceptado(int id) {
	Vuelos* temp = inicioV;
	while (temp != NULL) {
		if (temp->id == id) {
			temp->estatus = 1;
			break;
		}
		temp = temp->sig;
	}
}

void mostrarVuelo(int id)
{
	Vuelos* temp = inicioV;
	while (temp != NULL) {
		if (temp->id == id) {
			auxV = temp;
			break;
		}
		temp = temp->sig;
	}
}

void heapify(Vuelos* arr, int n, int i) {
	int largest = i;
	int left = 2 * i + 1;
	int right = 2 * i + 2;

	if (left < n && arr[left].id > arr[largest].id)
		largest = left;

	if (right < n && arr[right].id > arr[largest].id)
		largest = right;

	if (largest != i) {
		swap(arr[i], arr[largest]);
		heapify(arr, n, largest);
	}
}

void heapSort(Vuelos* arr, int n) {
	for (int i = n / 2 - 1; i >= 0; i--)
		heapify(arr, n, i);

	for (int i = n - 1; i >= 0; i--) {
		swap(arr[0], arr[i]);
		heapify(arr, i, 0);
	}
}



#pragma endregion

void guardarVuelos(Vuelos* inicioV) {
	std::ofstream file("vuelos.bin", std::ios::binary);
	Vuelos* current = inicioV;
	while (current != NULL) {
		file.write((char*)current, sizeof(Vuelos));
		current = current->sig;
	}
	file.close();
}

void cargarVuelos(Vuelos** inicioV) {
	std::ifstream file("vuelos.bin", std::ios::binary);
	Vuelos* current = new Vuelos;
	while (file.read((char*)current, sizeof(Vuelos))) {
		current->sig = *inicioV;
		if (*inicioV != NULL) {
			(*inicioV)->ant = current;
		}
		*inicioV = current;
		id++;
		current = new Vuelos;
	}
	delete current;
	file.close();
}

void leerArcVuelo()
{
	ifstream archivoV;
	archivoV.open("vuelos", ios::binary | ios::in);
	if (archivoV.is_open())
	{
		while (!archivoV.eof())
		{
			Vuelos* nuevo = new Vuelos;
			archivoV.read(reinterpret_cast<char*>(nuevo), sizeof(Vuelos));
			if (!archivoV.eof())
			{
				nuevo->sig = nullptr;
				nuevo->ant = nullptr;
				// Verificar si el nombre del piloto ya existe en la lista
				bool existe = false;
				Vuelos* temp = inicioV;
				while (temp != nullptr) {
					if (strcmp(temp->origen, nuevo->origen) == 0)
					{
						existe = true;
						break;
					}
					temp = temp->sig;
				}
				// Agregar el nuevo piloto solo si no existe en la lista
				if (!existe) {
					if (inicioV == nullptr) {
						inicioV = nuevo;
						id++;
					}
					else {
						Vuelos* aux = inicioV;
						while (aux->sig != nullptr) {
							aux = aux->sig;
						}
						aux->sig = nuevo;
						nuevo->ant = aux;
						id++;
					}
				}
				else {
					// Si el piloto ya existe, liberar la memoria del nuevo nodo
					delete nuevo;
				}
			}
		}
	}
	archivoV.close();
}

bool fechasCoinciden(const SYSTEMTIME& fecha1, const SYSTEMTIME& fecha2)
{
	return (fecha1.wYear == fecha2.wYear &&
		fecha1.wMonth == fecha2.wMonth &&
		fecha1.wDay == fecha2.wDay);
}

// Función para verificar si dos pilotos son iguales
bool pilotosIguales(const char* piloto1, const char* piloto2)
{
	return strcmp(piloto1, piloto2) == 0;
}

// Función para verificar si el piloto está disponible en la fecha de vuelo
bool pilotoDisponibleEnFecha(const char* piloto, const SYSTEMTIME& fechaVuelo, Vuelos* inicioV)
{
	Vuelos* temp = inicioV;
	while (temp != nullptr)
	{
		if (fechasCoinciden(fechaVuelo, convertirFecha(temp->fecha)) && pilotosIguales(piloto, temp->piloto)) {
			return false; // El piloto no está disponible en la fecha de vuelo
		}
		temp = temp->sig;
	}
	return true; // El piloto está disponible en la fecha de vuelo
}

//Normal
struct Pasajeros
{
	int id;
	char nombre[100];
	char apellidoP[100];
	char apellidoM[100];
	char nacimiento[100];
	bool genero;	//1 hombre,0 mujer
	char nacionalidad[100];
	char registro[100];
	char usuRegistro[100];
	int status;	//0 no tiene boleto, 1 cancelo, 2 abordo, 3 no abordo, 4 tiene vuelo pendiente, 5 "eliminado"
	int claveAsiento[100] = { 0 };
	int iDVuelo[100] = { 0 };//En cada espacio se le pone el id del vuelo
	int edad;
	Pasajeros* sig = NULL, * ant = NULL;
};	Pasajeros* inicioPa = NULL, * auxPa = NULL, * pivotePa = NULL;

void guardarPasajeros(Pasajeros* inicioPa) {
	std::ofstream file("pasajeros.bin", std::ios::binary);
	Pasajeros* current = inicioPa;
	while (current != NULL) {
		file.write((char*)current, sizeof(Pasajeros));
		current = current->sig;
	}
	file.close();
}

void cargarPasajeros(Pasajeros** inicioPa) {
	std::ifstream file("pasajeros.bin", std::ios::binary);
	Pasajeros* current = new Pasajeros;
	while (file.read((char*)current, sizeof(Pasajeros))) {
		current->sig = *inicioPa;
		if (*inicioPa != NULL) {
			(*inicioPa)->ant = current;
		}
		pasId++;
		*inicioPa = current;
		current = new Pasajeros;
	}
	delete current;
	file.close();
}

#pragma region Pasajeros
void agregarPasajero(Pasajeros*& inicio, Pasajeros* nuevo)
{
	if (inicio == NULL)
		inicio = nuevo;
	else {
		Pasajeros* aux = inicio;
		while (aux->sig != NULL)
			aux = aux->sig;
		aux->sig = nuevo;
		nuevo->ant = aux;
	}
}

void modificarPasajero(Pasajeros* nodo, const char* nombre, const char* apellidoP, const char* apellidoM, const char* nacionalidad) {
	strcpy_s(nodo->nombre, nombre);
	strcpy_s(nodo->apellidoP, apellidoP);
	strcpy_s(nodo->apellidoM, apellidoM);
	strcpy_s(nodo->nacionalidad, nacionalidad);
}

void eliminarPasajero(Pasajeros* nodo)
{
	nodo->status = 5;	//"Eliminado"
}
int busquedaBinariaPasajero(vector<int>& v, int id) {
	int izq = 0, der = v.size() - 1;
	while (izq <= der) {
		int m = izq + (der - 1) / 2;
		if (v[m] == id)
			return m;
		if (v[m] < id)
			izq = m + 1;
		else
			der = m - 1;
	}
	return -1;
}

Pasajeros* buscarPasajero(Pasajeros* inicio, int id)
{
	vector<int> ids;
	vector<Pasajeros*>nodos;
	Pasajeros* aux = inicio;
	while (aux != NULL) {
		ids.push_back(aux->id);
		nodos.push_back(aux);
		aux = aux->sig;
	}
	sort(ids.begin(), ids.end());
	int index = busquedaBinariaPasajero(ids, id);
	if (index != -1)
		return nodos[index];
}
int idPasajero(Pasajeros* inicio, const char* nombre) {
	auxPa = inicio;
	while (auxPa != NULL) {
		if (auxPa->status == 5)
			auxPa = auxPa->sig;
		else {
			if (strcmp(auxPa->nombre, nombre) == 0) {
				int cosa = auxPa->id;
				auxPa = inicioPa;
				return cosa;
			}
			else
				auxPa = auxPa->sig;
		}
	}
}
#pragma endregion

void guardarPasajero()
{
	ofstream archivoPa;
	archivoPa.open("pasajeros", ios::binary | ios::out | ios::app);
	if (archivoPa.is_open()) {
		Pasajeros* aux = inicioPa;
		while (aux != nullptr)
		{
			archivoPa.write(reinterpret_cast<char*>(aux), sizeof(Pasajeros));
			aux = aux->sig;
		}
	}
	archivoPa.close();
}

void leerArcPasajero()
{
	ifstream archivoPa;
	archivoPa.open("pasajeros", ios::binary | ios::in);
	if (archivoPa.is_open()) {
		while (!archivoPa.eof())
		{
			Pasajeros* nuevo = new Pasajeros;
			archivoPa.read(reinterpret_cast<char*>(nuevo), sizeof(Pasajeros));
			if (!archivoPa.eof())
			{
				nuevo->sig = nullptr;
				nuevo->ant = nullptr;
				// Verificar si el nombre del piloto ya existe en la lista
				bool existe = false;
				Pasajeros* temp = inicioPa;
				while (temp != nullptr)
				{
					if (strcmp(temp->nombre, nuevo->nombre) == 0) {
						existe = true;
						break;
					}
					temp = temp->sig;
				}
				// Agregar el nuevo piloto solo si no existe en la lista
				if (!existe) {
					if (inicioPa == nullptr) {
						inicioPa = nuevo;
					}
					else {
						Pasajeros* aux = inicioPa;
						while (aux->sig != nullptr)
						{
							aux = aux->sig;
						}
						aux->sig = nuevo;
						nuevo->ant = aux;
					}
				}
				else {
					// Si el piloto ya existe, liberar la memoria del nuevo nodo
					delete nuevo;
				}
			}
		}
	}
	archivoPa.close();
}

//Normal
struct Boleto
{
	int claveAsiento;
	int iDVuelo;
	bool tipo;	//0 turista, 1 ejecutivo
	int pago;	//0 credito, 1 debito, 2 efectivo
	int generacion;	//0 niño, 1 adulto, 2 anciano
	int status;	//0 comprado,1 abordo,2 no abordo, 3 cancelado
	Boleto* sig = NULL, * ant = NULL;
};	Boleto* inicioB = NULL, * auxB = NULL, * pivoteB = NULL;

#pragma region Boleto
void agregarBoleto(Boleto*& inicio, Boleto* nuevo)
{
	if (inicio == NULL)
		inicio = nuevo;
	else
	{
		Boleto* aux = inicioB;
		while (aux->sig != NULL)
			aux = aux->sig;
		aux->sig = nuevo;
		nuevo->ant = aux;
	}
}

void modificarBoleto(Boleto* nodo, int claveAsiento, int iDVuelo, bool tipo, int pago,
	int generacion, int status)
{
	nodo->claveAsiento, claveAsiento;
	nodo->iDVuelo, iDVuelo;
	nodo->tipo, tipo;
	nodo->pago, pago;
	nodo->generacion, generacion;
	nodo->status, status;
}

void guardarBoleto()
{
	ofstream archivoB;
	archivoB.open("boletos", ios::binary | ios::out | ios::app);
	if (archivoB.is_open())
	{
		Boleto* aux = inicioB;
		while (aux != nullptr)
		{
			archivoB.write(reinterpret_cast<char*>(aux), sizeof(Boleto));
			aux = aux->sig;
		}
	}
	archivoB.close();
}

void leerArcBoleto() {
	ifstream archivoB;
	archivoB.open("boletos", ios::binary | ios::in);
	if (archivoB.is_open()) {
		while (!archivoB.eof())
		{
			Boleto* nuevo = new Boleto;
			archivoB.read(reinterpret_cast<char*>(nuevo), sizeof(Boleto));
			if (!archivoB.eof())
			{
				nuevo->sig = nullptr;
				nuevo->ant = nullptr;
				// Verificar si el nombre del piloto ya existe en la lista
				bool existe = false;
				Boleto* temp = inicioB;
				while (temp != nullptr)
				{
					if ((temp->claveAsiento, nuevo->claveAsiento) == 0)
					{
						existe = true;
						break;
					}
					temp = temp->sig;
				}
				// Agregar el nuevo piloto solo si no existe en la lista
				if (!existe)
				{
					if (inicioB == nullptr) {
						inicioB = nuevo;
					}
					else {
						Boleto* aux = inicioB;
						while (aux->sig != nullptr)
						{
							aux = aux->sig;
						}
						aux->sig = nuevo;
						nuevo->ant = aux;
					}
				}
				else {
					// Si el piloto ya existe, liberar la memoria del nuevo nodo
					delete nuevo;
				}
			}
		}
	}
	archivoB.close();
}

#pragma endregion

void guardarBoletos(Boleto* inicioB) {
	std::ofstream file("boletos.bin", std::ios::binary);
	Boleto* current = inicioB;
	while (current != NULL) {
		file.write((char*)current, sizeof(Boleto));
		current = current->sig;
	}
	file.close();
}

void cargarBoletos(Boleto** inicioB) {
	std::ifstream file("boletos.bin", std::ios::binary);
	Boleto* current = new Boleto;
	while (file.read((char*)current, sizeof(Boleto))) {
		current->sig = *inicioB;
		if (*inicioB != NULL) {
			(*inicioB)->ant = current;
		}
		*inicioB = current;
		boletoId++;
		current = new Boleto;
	}
	delete current;
	file.close();
}

bool contieneNumero(const char* cadena)
{
	while (*cadena) {
		if (isdigit(*cadena)) {
			return true;
		}
		cadena++;
	}
	return false;
}

bool esNumero(const char* cadena)
{
	// Verifica si cada carácter en la cadena es un dígito numérico
	for (int i = 0; cadena[i] != '\0'; ++i) {
		if (!std::isdigit(cadena[i])) {
			return false; // No es un número
		}
	}
	return true; // Todos los caracteres son números
}

bool contieneCorreo(const char* cadena)
{
	if (strstr(cadena, "@") != NULL && strstr(cadena, ".com") != NULL)
		return true;
	return false;
}

bool contieneEstado(const char* cadena)
{
	if (strstr(cadena, "Disponible") != NULL || strstr(cadena, "Ocupado") != NULL)
		return true;
	return false;
}

bool contieneGenero(const char* cadena)
{
	if (strstr(cadena, "Masculino") != NULL || strstr(cadena, "Femenino") != NULL)
		return true;
	return false;
}

bool contieneBoleto(const char* cadena)
{
	if (strstr(cadena, "Con Boleto") != NULL || strstr(cadena, "Sin Boleto") != NULL)
		return true;
	return false;
}

LRESULT CALLBACK Inicio(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK menuPrincipal(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Usuarios(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Pilotos(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Vuelo(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Pasajero(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Compra(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CancelarCompra(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK Abordaje(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Incompletas
LRESULT CALLBACK Manifiesto(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hpControl;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow) {
	MSG mensaje;
	HWND hwnd = CreateDialog(hInstance, MAKEINTRESOURCE(InicioVentana), NULL, Inicio);
	//Lee los archivos 
	cargarPilotos(&inicioPilo);
	cargarVuelos(&inicioV);
	if (inicioPilo != NULL)
		quickSort(&inicioPilo);
	cargarPasajeros(&inicioPa);
	cargarBoletos(&inicioB);
	//leerArcPasajero();
	//leerArcVuelo();
	leerArcBoleto();
	/*leerArcUsuario();*/
	ShowWindow(hwnd, SW_SHOWDEFAULT);

	while (TRUE == GetMessage(&mensaje, 0, 0, 0)) {
		TranslateMessage(&mensaje);
		DispatchMessage(&mensaje);
	}
	guardarPilotos(inicioPilo);
	guardarVuelos(inicioV);
	guardarPasajeros(inicioPa);
	guardarBoletos(inicioB);

}
char recuerdame[10];
//Ventana de inicio

LRESULT CALLBACK Inicio(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	ifstream archivo("admin.txt");
	ifstream archivo2("recuerdame.txt");
	static HBRUSH pincel;
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		DestroyWindow(hwnd);
		break;
	case WM_CTLCOLORDLG:
		return (LRESULT)pincel;
	case WM_INITDIALOG:
		pincel = CreateSolidBrush(RGB(0, 5, 0));
		if (archivo.is_open())
		{
			string linea;
			getline(archivo, linea);
			strcpy_s(admin->apellidoM, linea.c_str());
			getline(archivo, linea);
			strcpy_s(admin->apellidoP, linea.c_str());
			getline(archivo, linea);
			strcpy_s(admin->contra, linea.c_str());
			getline(archivo, linea);
			strcpy_s(admin->correo, linea.c_str());
			getline(archivo, linea);
			strcpy_s(admin->foto, linea.c_str());
			getline(archivo, linea);
			if (linea == "0")
				admin->genero = 0;
			else
				admin->genero = 1;
			getline(archivo, linea);
			strcpy_s(admin->nacimiento, linea.c_str());
			getline(archivo, linea);
			strcpy_s(admin->nombre, linea.c_str());
			getline(archivo, linea);
			strcpy_s(admin->registro, linea.c_str());
			getline(archivo, linea);
			strcpy_s(admin->usuario, linea.c_str());
			getline(archivo, linea);
			strcpy_s(admin->usuRegistro, linea.c_str());
		}
		archivo.close();

		if (archivo2.is_open()) {
			string linea;
			getline(archivo2, linea);
			strcpy_s(pivoteU->apellidoM, linea.c_str());
			getline(archivo2, linea);
			strcpy_s(pivoteU->apellidoP, linea.c_str());
			getline(archivo2, linea);
			strcpy_s(pivoteU->contra, linea.c_str());
			getline(archivo2, linea);
			strcpy_s(pivoteU->correo, linea.c_str());
			getline(archivo2, linea);
			strcpy_s(pivoteU->foto, linea.c_str());
			getline(archivo2, linea);
			if (linea == "0")
				pivoteU->genero = 0;
			else
				pivoteU->genero = 1;
			getline(archivo2, linea);
			strcpy_s(pivoteU->nacimiento, linea.c_str());
			getline(archivo2, linea);
			strcpy_s(pivoteU->nombre, linea.c_str());
			getline(archivo2, linea);
			strcpy_s(pivoteU->registro, linea.c_str());
			getline(archivo2, linea);
			strcpy_s(pivoteU->usuario, linea.c_str());
			getline(archivo2, linea);
			strcpy_s(pivoteU->usuRegistro, linea.c_str());
			getline(archivo2, linea);
			strcpy_s(recuerdame, linea.c_str());
		}
		archivo2.close();
		if (strcmp(recuerdame, "1") == 0) {
			SetDlgItemText(hwnd, cuadroUsuario, pivoteU->usuario);
			SetDlgItemText(hwnd, cuadroContra, pivoteU->contra);
			SendDlgItemMessage(hwnd, RECUERDAME, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

		}
		//Esto es lo que provoca que guarde a silver tambien, porque jala todo lo de la lista otra vez
		/*pivoteU = new User();
		inicioU = insertarUsuario(inicioU, admin);*/
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case INGRESAR:
			GetDlgItemText(hwnd, cuadroUsuario, pivoteU->usuario, 100);
			GetDlgItemText(hwnd, cuadroContra, pivoteU->contra, 100);
			if (strcmp(pivoteU->usuario, admin->usuario) != 0) {
				buscarUsuario(pivoteU->usuario);
				if (auxU != NULL)
					if (strcmp(auxU->contra, pivoteU->contra) == 0) {
						actual = auxU;
						ofstream archivo("recuerdame.txt");
						if (archivo.is_open()) {
							archivo << auxU->apellidoM << endl;
							archivo << auxU->apellidoP << endl;
							archivo << auxU->contra << endl;
							archivo << auxU->correo << endl;
							archivo << auxU->foto << endl;
							archivo << auxU->genero << endl;
							archivo << auxU->nacimiento << endl;
							archivo << auxU->nombre << endl;
							archivo << auxU->registro << endl;
							archivo << auxU->usuario << endl;
							archivo << auxU->usuRegistro << endl;
							if (IsDlgButtonChecked(hwnd, RECUERDAME) == BST_CHECKED)
								archivo << "1" << endl;
							else
								archivo << "0" << endl;
						}
						archivo.close();
						DialogBox(NULL, MAKEINTRESOURCE(menuVentana), hwnd, (DLGPROC)menuPrincipal);

					}
					else
						MessageBox(hwnd, "Contraseña incorrecta", "Aviso", MB_OK);
				else
					MessageBox(hwnd, "Usuario no encontrado", "Aviso", MB_OK);
			}
			else {
				if (strcmp(admin->contra, pivoteU->contra) == 0) {
					actual = admin;
					ofstream archivo("recuerdame.txt");
					if (archivo.is_open()) {
						archivo << actual->apellidoM << endl;
						archivo << actual->apellidoP << endl;
						archivo << actual->contra << endl;
						archivo << actual->correo << endl;
						archivo << actual->foto << endl;
						archivo << actual->genero << endl;
						archivo << actual->nacimiento << endl;
						archivo << actual->nombre << endl;
						archivo << actual->registro << endl;
						archivo << actual->usuario << endl;
						archivo << actual->usuRegistro << endl;
						if (IsDlgButtonChecked(hwnd, RECUERDAME) == BST_CHECKED)
							archivo << "1" << endl;
						else
							archivo << "0" << endl;
					}
					archivo.close();
					DialogBox(NULL, MAKEINTRESOURCE(menuVentana), hwnd, (DLGPROC)menuPrincipal);
				}
				else
					MessageBox(hwnd, "Contraseña incorrecta", "Aviso", MB_OK);
			}
			break;
		}
		break;
	}
	return 0;
}

/*
	//Ventana de menu principal

	Registrar:
	Usuarios
	Pilotos
	Vuelos (cancelar en vez de eliminar)
	Pasajeros (Si se elimina, se cancela su boleto)
	Boletos (En vez de eliminar, cancelar pero en una ventana aparte)

	Abordo (Si entro o no al vuelo)
	Vuelos (Si esta volando o cancelado, si se cancela
			cancelar todos los boletos)
	Cancelar Boleto
	Manifiesto


		compra boletos
		cancelar boletos
		detalles vuelo (cancelarlo o confirmarlo, no los modifica)
		registro de vuelo
		usuarios
		pases de abordo
		manifiesto

*/

LRESULT CALLBACK menuPrincipal(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HBRUSH pincel;
	switch (msg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_CTLCOLORDLG:
		return (LRESULT)pincel;
	case WM_INITDIALOG:
		pincel = CreateSolidBrush(RGB(5, 105, 255));
		//mandar imagen
		hpControl = GetDlgItem(hwnd, FOTOUSUARIO);
		HBITMAP bmp;
		bmp = (HBITMAP)LoadImage(NULL, actual->foto, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
		SendMessage(hpControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);
		SetDlgItemText(hwnd, IDC_EDIT1, actual->usuario);

		if (actual == admin)
			EnableWindow(GetDlgItem(hwnd, USUARIOS), true);
		else
			EnableWindow(GetDlgItem(hwnd, USUARIOS), false);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case USUARIOS:
			DialogBox(NULL, MAKEINTRESOURCE(usuarioVentana), hwnd, (DLGPROC)Usuarios);
			break;
		case PILOTOS:
			DialogBox(NULL, MAKEINTRESOURCE(ventanaPilotos), hwnd, (DLGPROC)Pilotos);
			break;
		case VUELOS:
			DialogBox(NULL, MAKEINTRESOURCE(vueloVentana), hwnd, (DLGPROC)Vuelo);
			break;
		case PASAJEROS:
			DialogBox(NULL, MAKEINTRESOURCE(pasajeroVentana), hwnd, (DLGPROC)Pasajero);
			break;
		case COMPRABOLETOS:
			DialogBox(NULL, MAKEINTRESOURCE(ventanaCompra), hwnd, (DLGPROC)Compra);
			break;
		case CANCELARBOLETO:
			DialogBox(NULL, MAKEINTRESOURCE(ventanaCancelacion), hwnd, (DLGPROC)CancelarCompra);
			break;
		case ABORDAJE:
			DialogBox(NULL, MAKEINTRESOURCE(ventanaAbordaje), hwnd, (DLGPROC)Abordaje);
			break;
		case MANIFIESTO:
			DialogBox(NULL, MAKEINTRESOURCE(ventanaManifiesto), hwnd, (DLGPROC)Manifiesto);
			break;
		case SALIR:
			PostQuitMessage(117);
			break;
		}
		break;
	}
	return 0;
}

SYSTEMTIME systemTime;
char fotografia[100];
char usuario[100];

//Listo
//Modificar
//Eliminar
LRESULT CALLBACK Usuarios(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HBRUSH pincel;
	pivoteU = new User();
	switch (msg) {
	case WM_CLOSE:
		EndDialog(hwnd, LOWORD(wParam));

		break;
	case WM_CTLCOLORDLG:
		return (LRESULT)pincel;
	case WM_INITDIALOG:
		pincel = CreateSolidBrush(RGB(5, 105, 255));
		SetDlgItemText(hwnd, IDC_EDIT1, actual->usuario);

		auxU = inicioU;
		SendMessage(GetDlgItem(hwnd, LISTAUSUARIOS), LB_RESETCONTENT, 0, 0);
		llenarUsuarios(hwnd, auxU);

		SetDlgItemText(hwnd, ADMIN, actual->usuario);
		//mandar imagen
		hpControl = GetDlgItem(hwnd, ADMINFOTO);
		HBITMAP bmp;
		bmp = (HBITMAP)LoadImage(NULL, actual->foto, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
		SendMessage(hpControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);

		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case LISTAUSUARIOS:
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
				EnableWindow(GetDlgItem(hwnd, MODIFICARUSU), true);
				EnableWindow(GetDlgItem(hwnd, ELIMINARUSU), true);
				EnableWindow(GetDlgItem(hwnd, REGISTRARUSU), false);
				EnableWindow(GetDlgItem(hwnd, USUARIO), false);
				EnableWindow(GetDlgItem(hwnd, RHOMBRE), false);
				EnableWindow(GetDlgItem(hwnd, RMUJER), false);
				EnableWindow(GetDlgItem(hwnd, NACIMIENTO), false);
				int indice;
				indice = SendDlgItemMessage(hwnd, LISTAUSUARIOS, LB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hwnd, LISTAUSUARIOS, LB_GETTEXT, indice, (LPARAM)usuario);
				buscarUsuario(usuario);
				SetDlgItemText(hwnd, USUARIO, auxU->usuario);
				SetDlgItemText(hwnd, USUREGISTRO, auxU->usuRegistro);
				SetDlgItemText(hwnd, NOMBRE, auxU->nombre);
				SetDlgItemText(hwnd, APELLIDOP, auxU->apellidoP);
				SetDlgItemText(hwnd, APELLIDOM, auxU->apellidoM);
				SetDlgItemText(hwnd, CORREO, auxU->correo);
				SetDlgItemText(hwnd, CONTRA, auxU->contra);
				hpControl = GetDlgItem(hwnd, FOTOUSUARIO);

				if (auxU->genero == 1)
					SendDlgItemMessage(hwnd, RHOMBRE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				else
					SendDlgItemMessage(hwnd, RMUJER, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

				HBITMAP bmp;
				bmp = (HBITMAP)LoadImage(NULL, auxU->foto, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
				SendMessage(hpControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);

				SYSTEMTIME st;
				st = convertirFecha(auxU->nacimiento);
				SendDlgItemMessage(hwnd, NACIMIENTO, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&st);
				st = convertirFecha(auxU->registro);
				SendDlgItemMessage(hwnd, REGISTRO, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&st);
				break;
			}
			break;

		case AGREGARFOTO:
			//nope, no guarda :c
			OPENFILENAME fotoUsu;
			ZeroMemory(&fotoUsu, sizeof(fotoUsu));

			strcpy_s(fotografia, "");

			fotoUsu.hwndOwner = hwnd;
			fotoUsu.lpstrFile = fotografia;
			fotoUsu.lStructSize = sizeof(OPENFILENAME);
			fotoUsu.nMaxFile = MAX_PATH;
			fotoUsu.lpstrDefExt = "txt";
			fotoUsu.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;
			fotoUsu.lpstrFilter = "*.bmp \0 *.*\0";
			if (GetOpenFileName(&fotoUsu)) {
				HWND hPControl = GetDlgItem(hwnd, FOTOUSUARIO);
				HBITMAP hImagen = (HBITMAP)LoadImage(NULL, fotografia, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
				SendMessage(hPControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hImagen);
			}
			break;

		case REGISTRARUSU:
			pivoteU = new User();
			//datos
			GetDlgItemText(hwnd, USUARIO, pivoteU->usuario, 100);
			if (nombreRepetidoU(pivoteU->usuario, inicioU))
			{
				MessageBox(NULL, "El nombre del usuairo ya está en uso.", "Error", MB_OK | MB_ICONERROR);
				break;
			}

			GetDlgItemText(hwnd, NOMBRE, pivoteU->nombre, 100);
			if (contieneNumero(pivoteU->nombre))
				strcpy_s(pivoteU->nombre, "\0");

			GetDlgItemText(hwnd, APELLIDOP, pivoteU->apellidoP, 100);
			if (contieneNumero(pivoteU->apellidoP))
				strcpy_s(pivoteU->apellidoP, "\0");

			GetDlgItemText(hwnd, APELLIDOM, pivoteU->apellidoM, 100);
			if (contieneNumero(pivoteU->apellidoM))
				strcpy_s(pivoteU->apellidoM, "\0");

			GetDlgItemText(hwnd, CORREO, pivoteU->correo, 100);
			if (!contieneCorreo(pivoteU->correo))
				strcpy_s(pivoteU->correo, "\0");

			GetDlgItemText(hwnd, CONTRA, pivoteU->contra, 100);

			GetDlgItemText(hwnd, ADMIN, pivoteU->usuRegistro, 100);

			//Segun yo aquí no va nada de la fotografía
			strcpy_s(pivoteU->foto, fotografia);

			if (IsDlgButtonChecked(hwnd, RHOMBRE) == BST_CHECKED)
				pivoteU->genero = 1;
			if (IsDlgButtonChecked(hwnd, RMUJER) == BST_CHECKED)
				pivoteU->genero = 0;

			//fechas

			SendDlgItemMessage(hwnd, REGISTRO, DTM_GETSYSTEMTIME, 0, (LPARAM)&systemTime);
			snprintf(pivoteU->registro, sizeof(pivoteU->registro), "%02d/%02d/%04d %02d:%02d:%02d",
				systemTime.wMonth, systemTime.wDay, systemTime.wYear,
				systemTime.wHour, systemTime.wMinute, systemTime.wSecond);

			SYSTEMTIME systemTime;
			SendDlgItemMessage(hwnd, NACIMIENTO, DTM_GETSYSTEMTIME, 0, (LPARAM)&systemTime);
			snprintf(pivoteU->nacimiento, sizeof(pivoteU->nacimiento), "%02d/%02d/%04d %02d:%02d:%02d",
				systemTime.wMonth, systemTime.wDay, systemTime.wYear,
				systemTime.wHour, systemTime.wMinute, systemTime.wSecond);

			//Por alguna razón no funciona ya que no registra bien si se usa

			if (pivoteU->nombre[0] != NULL && pivoteU->apellidoP[0] != NULL
				&& pivoteU->apellidoM[0] != NULL && pivoteU->correo[0] != NULL)
			{
				//Registra al usuario en el arbol
				if (esMayorDe18(pivoteU->nacimiento))
				{
					// El usuario es mayor de 18 años
					if (inicioU == NULL)
					{
						inicioU = insertarUsuario(inicioU, pivoteU);
						guardarUsuarios();
					}
					else
					{
						insertarUsuario(inicioU, pivoteU);
						guardarUsuarios();
					}

					auxU = inicioU;

					SendMessage(GetDlgItem(hwnd, LISTAUSUARIOS), LB_RESETCONTENT, 0, 0);

					llenarUsuariosR(hwnd, auxU);

					limpiarVentanaUsuario(hwnd);

				}
				else
				{
					MessageBox(NULL, "El usuario no es mayor de edad.", "Error", MB_OK | MB_ICONERROR);
					break; 
				}

				MessageBox(NULL, "Registro hecho correctamente", "USUARIO", MB_OK);
			}
			else
			{
				MessageBox(NULL, "Registro invalido", "USUARIO", MB_OK);
			}
			break;

		case MODIFICARUSU:
			//datos
			SendDlgItemMessage(hwnd, RMUJER, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, RHOMBRE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			GetDlgItemText(hwnd, NOMBRE, pivoteU->nombre, 100);
			GetDlgItemText(hwnd, APELLIDOP, pivoteU->apellidoP, 100);
			GetDlgItemText(hwnd, APELLIDOM, pivoteU->apellidoM, 100);
			GetDlgItemText(hwnd, CORREO, pivoteU->correo, 100);
			GetDlgItemText(hwnd, CONTRA, pivoteU->contra, 100);
			strcpy_s(pivoteU->foto, fotografia);
			//fechas
			SendDlgItemMessage(hwnd, NACIMIENTO, DTM_GETSYSTEMTIME, 0, (LPARAM)&systemTime);
			snprintf(pivoteU->nacimiento, sizeof(pivoteU->nacimiento), "%02d/%02d/%04d %02d:%02d:%02d",
				systemTime.wMonth, systemTime.wDay, systemTime.wYear,
				systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
			pivoteU->genero = auxU->genero;
			modificarUsuario(auxU, pivoteU);
			limpiarVentanaUsuario(hwnd);
			EnableWindow(GetDlgItem(hwnd, MODIFICARUSU), false);
			EnableWindow(GetDlgItem(hwnd, ELIMINARUSU), false);
			EnableWindow(GetDlgItem(hwnd, REGISTRARUSU), true);
			EnableWindow(GetDlgItem(hwnd, USUARIO), true);
			EnableWindow(GetDlgItem(hwnd, RHOMBRE), true);
			EnableWindow(GetDlgItem(hwnd, RMUJER), true);
			//validacion aqui

			break;
		case ELIMINARUSU:
			GetDlgItemText(hwnd, USUARIO, pivoteU->usuario, 100);
			eliminarUsuario(pivoteU->usuario);
			guardarUsuarios();
			limpiarVentanaUsuario(hwnd);

			auxU = inicioU;
			SendMessage(GetDlgItem(hwnd, LISTAUSUARIOS), LB_RESETCONTENT, 0, 0);
			llenarUsuariosR(hwnd, auxU);

			limpiarVentanaUsuario(hwnd);

			EnableWindow(GetDlgItem(hwnd, MODIFICARUSU), false);
			EnableWindow(GetDlgItem(hwnd, ELIMINARUSU), false);
			EnableWindow(GetDlgItem(hwnd, REGISTRARUSU), true);
			EnableWindow(GetDlgItem(hwnd, USUARIO), true);
			EnableWindow(GetDlgItem(hwnd, RHOMBRE), true);
			EnableWindow(GetDlgItem(hwnd, RMUJER), true);

			break;
		}
		break;
	}
	return 0;
}

//Listo
//Modificar
//Eliminar
LRESULT CALLBACK Pilotos(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HBRUSH pincel;
	switch (msg)
	{
	case WM_CLOSE:
		EndDialog(hwnd, LOWORD(wParam));

		break;
	case WM_CTLCOLORDLG:
		return (LRESULT)pincel;
	case WM_INITDIALOG:
		pincel = CreateSolidBrush(RGB(5, 105, 255));
		//mandar imagen
		hpControl = GetDlgItem(hwnd, FOTOUSUARIO);
		HBITMAP bmp;
		bmp = (HBITMAP)LoadImage(NULL, actual->foto, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
		SendMessage(hpControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);
		SetDlgItemText(hwnd, IDC_EDIT1, actual->usuario);
		auxP = inicioPilo;
		SendMessage(GetDlgItem(hwnd, LISTAPILO), LB_RESETCONTENT, 0, 0);
		while (auxP != NULL) {
			SendMessage(GetDlgItem(hwnd, LISTAPILO), LB_ADDSTRING, 0, (LPARAM)auxP->nombre);
			auxP = auxP->sig;
		}
		SetDlgItemText(hwnd, USUARIO, actual->usuario);
		//falta mandar imagen
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case LISTAPILO: {
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE: {
				EnableWindow(GetDlgItem(hwnd, REGISTRARPILO), false);
				EnableWindow(GetDlgItem(hwnd, MODIFICARPILO), true);
				EnableWindow(GetDlgItem(hwnd, ELIMINARPILO), true);
				int indice;
				indice = SendDlgItemMessage(hwnd, LISTAPILO, LB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hwnd, LISTAPILO, LB_GETTEXT, indice, (LPARAM)usuario);
				mostrarPiloto(inicioPilo, usuario);
				SetDlgItemText(hwnd, NOMBRE, auxP->nombre);
				SetDlgItemText(hwnd, APELLIDOP, auxP->apellidoP);
				SetDlgItemText(hwnd, APELLIDOM, auxP->apellidoM);
				SetDlgItemInt(hwnd, HORAS, auxP->horas, 0);
				break;
			}
			}

			// Limpiar la ListBox y llenarla solo una vez
			if (HIWORD(wParam) != LBN_SELCHANGE)
			{
				SendMessage(GetDlgItem(hwnd, LISTAPILO), LB_RESETCONTENT, 0, 0);

				//// Llamar a la función para leer los datos del archivo de pilotos solo una vez
				//leerArcPiloto();

				// Agregar los elementos a la ListBox solo una vez
				Piloto* temp = inicioPilo;
				while (temp != nullptr) {
					SendMessage(GetDlgItem(hwnd, LISTAPILO), LB_ADDSTRING, 0, (LPARAM)temp->nombre);
					temp = temp->sig;
				}
			}
			break;
		}

		case REGISTRARPILO:
			pivoteP = new Piloto();
			BOOL a;
			GetDlgItemText(hwnd, NOMBRE, pivoteP->nombre, 100);
			if (nombreRepetidoP(pivoteP->nombre, inicioPilo))
			{
				MessageBox(NULL, "El nombre del piloto ya está en uso.", "Error", MB_OK | MB_ICONERROR);
				break;
			}
			if (contieneNumero(pivoteP->nombre))
				strcpy_s(pivoteP->nombre, "\0");

			GetDlgItemText(hwnd, APELLIDOP, pivoteP->apellidoP, 100);
			if (contieneNumero(pivoteP->apellidoP))
				strcpy_s(pivoteP->apellidoP, "\0");
			GetDlgItemText(hwnd, APELLIDOM, pivoteP->apellidoM, 100);
			if (contieneNumero(pivoteP->apellidoM))
				strcpy_s(pivoteP->apellidoM, "\0");

			pivoteP->horas = GetDlgItemInt(hwnd, HORAS, &a, false);

			if (pivoteP->nombre[0] != NULL && pivoteP->apellidoP[0] != NULL
				&& pivoteP->apellidoM[0] != NULL)
			{
				MessageBox(NULL, "Registro hecho correctamente", "PILOTOS", MB_OK);
				agregarPiloto(&inicioPilo, pivoteP->nombre, pivoteP->apellidoP, pivoteP->apellidoM, pivoteP->horas);
				SendMessage(GetDlgItem(hwnd, LISTAPILO), LB_RESETCONTENT, 0, 0);
				auxP = inicioPilo;
				quickSort(&inicioPilo);
				while (auxP != NULL)
				{
					SendMessage(GetDlgItem(hwnd, LISTAPILO), LB_ADDSTRING, 0, (LPARAM)auxP->nombre);
					auxP = auxP->sig;
				}
				//

				SetDlgItemText(hwnd, NOMBRE, 0);
				SetDlgItemText(hwnd, APELLIDOP, 0);
				SetDlgItemText(hwnd, APELLIDOM, 0);
				SetDlgItemInt(hwnd, HORAS, 0, 0);
			}
			else
			{
				MessageBox(NULL, "Registro invalido", "PILOTOS", MB_OK);
			}

			break;
		case MODIFICARPILO:
			EnableWindow(GetDlgItem(hwnd, REGISTRARPILO), true);
			EnableWindow(GetDlgItem(hwnd, MODIFICARPILO), false);
			EnableWindow(GetDlgItem(hwnd, ELIMINARPILO), false);
			pivoteP = new Piloto();
			GetDlgItemText(hwnd, NOMBRE, pivoteP->nombre, 100);
			if (contieneNumero(pivoteP->nombre))
				strcpy_s(pivoteP->nombre, "\0");
			GetDlgItemText(hwnd, APELLIDOP, pivoteP->apellidoP, 100);
			if (contieneNumero(pivoteP->apellidoP))
				strcpy_s(pivoteP->apellidoP, "\0");
			GetDlgItemText(hwnd, APELLIDOM, pivoteP->apellidoM, 100);
			if (contieneNumero(pivoteP->apellidoM))
				strcpy_s(pivoteP->apellidoM, "\0");

			pivoteP->horas = GetDlgItemInt(hwnd, HORAS, &a, false);

			if (pivoteP->nombre[0] != NULL && pivoteP->apellidoP[0] != NULL
				&& pivoteP->apellidoM[0] != NULL)
			{
				modificarPiloto(inicioPilo, usuario, pivoteP->nombre, pivoteP->apellidoP, pivoteP->apellidoM, pivoteP->horas);

				auxP = inicioPilo;
				quickSort(&inicioPilo);
				SendMessage(GetDlgItem(hwnd, LISTAPILO), LB_RESETCONTENT, 0, 0);
				while (auxP != NULL) {
					SendMessage(GetDlgItem(hwnd, LISTAPILO), LB_ADDSTRING, 0, (LPARAM)auxP->nombre);
					auxP = auxP->sig;
				}

				SetDlgItemText(hwnd, NOMBRE, 0);
				SetDlgItemText(hwnd, APELLIDOP, 0);
				SetDlgItemText(hwnd, APELLIDOM, 0);
				SetDlgItemInt(hwnd, HORAS, 0, 0);
			}
			else
			{
				MessageBox(NULL, "Registro invalido", "USUARIO", MB_OK);
			}
			break;

		case ELIMINARPILO:
			EnableWindow(GetDlgItem(hwnd, REGISTRARPILO), true);
			EnableWindow(GetDlgItem(hwnd, MODIFICARPILO), false);
			EnableWindow(GetDlgItem(hwnd, ELIMINARPILO), false);
			pivoteP = new Piloto();
			GetDlgItemText(hwnd, NOMBRE, pivoteP->nombre, 100);
			GetDlgItemText(hwnd, APELLIDOP, pivoteP->apellidoP, 100);
			GetDlgItemText(hwnd, APELLIDOM, pivoteP->apellidoM, 100);
			pivoteP->horas = GetDlgItemInt(hwnd, HORAS, &a, false);
			eliminarPiloto(&inicioPilo, usuario);
			auxP = inicioPilo;
			if (inicioPilo != NULL)
				quickSort(&inicioPilo);
			SendMessage(GetDlgItem(hwnd, LISTAPILO), LB_RESETCONTENT, 0, 0);
			while (auxP != NULL)
			{
				SendMessage(GetDlgItem(hwnd, LISTAPILO), LB_ADDSTRING, 0, (LPARAM)auxP->nombre);
				auxP = auxP->sig;
			}
			//Podria ir guardar Piloto 
			SetDlgItemText(hwnd, NOMBRE, 0);
			SetDlgItemText(hwnd, APELLIDOP, 0);
			SetDlgItemText(hwnd, APELLIDOM, 0);
			SetDlgItemInt(hwnd, HORAS, 0, 0);
			break;
		}
		break;
	}
	return 0;
}
int i = 0;

//Listo
//Validar Fecha de Registro
LRESULT CALLBACK Vuelo(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HBRUSH pincel;
	switch (msg)
	{
	case WM_CLOSE:
		//guardarVuelo();
		EndDialog(hwnd, LOWORD(wParam));
		break;
	case WM_CTLCOLORDLG:
		return (LRESULT)pincel;
	case WM_INITDIALOG:
		pincel = CreateSolidBrush(RGB(5, 105, 255));
		//mandar imagen
		hpControl = GetDlgItem(hwnd, FOTOUSUARIO);
		HBITMAP bmp;
		bmp = (HBITMAP)LoadImage(NULL, actual->foto, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
		SendMessage(hpControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);
		SetDlgItemText(hwnd, IDC_EDIT1, actual->usuario);

		auxV = inicioV;
		SendMessage(GetDlgItem(hwnd, LISTAVUELOS), LB_RESETCONTENT, 0, 0);
		while (auxV != NULL)
		{
			char temp[100];
			strcpy_s(temp, to_string(auxV->id).c_str());
			SendMessage(GetDlgItem(hwnd, LISTAVUELOS), LB_ADDSTRING, 0, (LPARAM)temp);
			auxV = auxV->sig;
		}
		SetDlgItemText(hwnd, ADMIN, actual->usuario);
		SendMessage(GetDlgItem(hwnd, ESTATUS), CB_INSERTSTRING, (WPARAM)0, (LPARAM)"REGISTRADO");
		SendMessage(GetDlgItem(hwnd, ESTATUS), CB_INSERTSTRING, (WPARAM)1, (LPARAM)"ABORDADO");
		SendMessage(GetDlgItem(hwnd, ESTATUS), CB_INSERTSTRING, (WPARAM)2, (LPARAM)"CANCELADO");
		i = 0;
		auxP = inicioPilo;
		while (auxP != NULL) {
			if (auxP->status == true)
			{
				SendMessage(GetDlgItem(hwnd, PILOTO), CB_INSERTSTRING, (WPARAM)i, (LPARAM)auxP->nombre);
				i++;
			}
			auxP = auxP->sig;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case LISTAVUELOS:
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
				auxV = inicioV;
				EnableWindow(GetDlgItem(hwnd, REGISTRAR), false);
				EnableWindow(GetDlgItem(hwnd, ORIGEN), false);
				EnableWindow(GetDlgItem(hwnd, DESTINO), false);
				EnableWindow(GetDlgItem(hwnd, FECHAVUELO), false);
				EnableWindow(GetDlgItem(hwnd, FECHAREGISTRO), false);
				EnableWindow(GetDlgItem(hwnd, PILOTO), false);
				EnableWindow(GetDlgItem(hwnd, RANCHO), false);
				EnableWindow(GetDlgItem(hwnd, RESTRECHO), false);
				EnableWindow(GetDlgItem(hwnd, ASIENTOS), false);
				EnableWindow(GetDlgItem(hwnd, NINIOS), false);
				EnableWindow(GetDlgItem(hwnd, ADULTOS), false);
				EnableWindow(GetDlgItem(hwnd, CANCELAR), false);
				EnableWindow(GetDlgItem(hwnd, ABORDAR), false);
				int indice;
				int idNow;
				indice = SendDlgItemMessage(hwnd, LISTAVUELOS, LB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hwnd, LISTAVUELOS, LB_GETTEXT, indice, (LPARAM)usuario);
				idNow = stoi(usuario);
				mostrarVuelo(idNow);
				SetDlgItemText(hwnd, ORIGEN, auxV->origen);
				SetDlgItemText(hwnd, DESTINO, auxV->destino);

				SYSTEMTIME st;
				st = convertirFecha(auxV->fecha);
				SendDlgItemMessage(hwnd, FECHAVUELO, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&st);
				st = convertirFecha(auxV->registro);
				SendDlgItemMessage(hwnd, FECHAREGISTRO, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&st);

				SetDlgItemText(hwnd, USUREGISTRO, auxV->usuRegistro);

				SetDlgItemText(hwnd, PILOTO, auxV->piloto);
				//SendMessage(GetDlgItem(hwnd, PILOTO), CB_SELECTSTRING, 0, (LPARAM)auxV->piloto);

				SendDlgItemMessage(hwnd, RANCHO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
				SendDlgItemMessage(hwnd, RESTRECHO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
				SetDlgItemInt(hwnd, ID, auxV->id, 0);
				if (auxV->tipoAvion == 0)//ancho
					SendDlgItemMessage(hwnd, RANCHO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				else
					SendDlgItemMessage(hwnd, RESTRECHO, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

				SetDlgItemInt(hwnd, ASIENTOS, auxV->asientos, 0);
				SetDlgItemInt(hwnd, NINIOS, auxV->childPercent, 0);
				SetDlgItemInt(hwnd, ADULTOS, auxV->oldPercent, 0);
				if (auxV->estatus == 0)
					SendMessage(GetDlgItem(hwnd, ESTATUS), CB_SELECTSTRING, 0, (LPARAM)"REGISTRADO");
				if (auxV->estatus == 1)
					SetDlgItemText(hwnd, ESTATUS, "DESPEGO");
				//SendMessage(GetDlgItem(hwnd, ESTATUS), CB_SELECTSTRING, 0, (LPARAM)"DESPEGO");
				if (auxV->estatus == 2)
					SendMessage(GetDlgItem(hwnd, ESTATUS), CB_SELECTSTRING, 0, (LPARAM)"CANCELADO");
				if (auxV->estatus == 0) {
					EnableWindow(GetDlgItem(hwnd, CANCELAR), true);
					EnableWindow(GetDlgItem(hwnd, ABORDAR), true);
				}
				break;
			}
			break;
		case REGISTRAR:
		{
			pivoteV = new Vuelos();
			GetDlgItemText(hwnd, ORIGEN, pivoteV->origen, 100);
			if (contieneNumero(pivoteV->origen))
				strcpy_s(pivoteV->origen, "\0");
			GetDlgItemText(hwnd, DESTINO, pivoteV->destino, 100);
			if (contieneNumero(pivoteV->destino))
				strcpy_s(pivoteV->destino, "\0");

			if (pivoteV->origen[0] != NULL && pivoteV->destino[0] != NULL)
			{
				//fecha
				SYSTEMTIME systemTime;
				SendDlgItemMessage(hwnd, FECHAVUELO, DTM_GETSYSTEMTIME, 0, (LPARAM)&systemTime);
				if (seisMeses(systemTime) == true) {
					MessageBox(NULL, "El vuelo tiene que ser dentro de los 6 meses disponibles", "VUELOS", MB_OK);
					break;
				}


				snprintf(pivoteV->fecha, sizeof(pivoteV->fecha), "%02d/%02d/%04d %02d:%02d:%02d",
					systemTime.wMonth, systemTime.wDay, systemTime.wYear,
					systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
				GetSystemTime(&systemTime);
				snprintf(pivoteV->registro, sizeof(pivoteV->registro), "%02d/%02d/%04d %02d:%02d:%02d",
					systemTime.wMonth, systemTime.wDay, systemTime.wYear,
					systemTime.wHour, systemTime.wMinute, systemTime.wSecond);

				GetDlgItemText(hwnd, ADMIN, pivoteV->usuRegistro, 100);

				SendDlgItemMessage(hwnd, PILOTO, WM_GETTEXT, 100, (LPARAM)pivoteV->piloto);

				// Verificar si el piloto ya está programado para volar en la misma fecha
				if (!pilotoDisponibleEnFecha(pivoteV->piloto, convertirFecha(pivoteV->fecha), inicioV))
				{
					MessageBox(hwnd, "El piloto ya está programado para volar en esa misma fecha.", "Error", MB_OK | MB_ICONERROR);
					break;
				}

				mostrarPiloto(inicioPilo, pivoteV->piloto);
				auxP->status = false;
				//No esta jalando pivoteV->id, porque id no aumenta
				id++;
				pivoteV->id = id;

				if (IsDlgButtonChecked(hwnd, RANCHO) == BST_CHECKED)
					pivoteV->tipoAvion = 0;
				if (IsDlgButtonChecked(hwnd, RESTRECHO) == BST_CHECKED)
					pivoteV->tipoAvion = 1;

				BOOL a;
				pivoteV->asientos = GetDlgItemInt(hwnd, ASIENTOS, &a, false);
				pivoteV->childPercent = pivoteV->asientos * .1;
				pivoteV->childPercent = floor(pivoteV->childPercent);
				pivoteV->oldPercent = pivoteV->asientos * .15;
				pivoteV->oldPercent = floor(pivoteV->oldPercent);
				pivoteV->estatus = 0;
				pivoteV->totalAsientos = pivoteV->asientos;
				pivoteV->totalChild = pivoteV->childPercent;
				pivoteV->totalOld = pivoteV->oldPercent;
				insertarVuelo(pivoteV);

				MessageBox(NULL, "Registro hecho correctamente", "VUELOS", MB_OK);

				auxV = inicioV;
				SendMessage(GetDlgItem(hwnd, LISTAVUELOS), LB_RESETCONTENT, 0, 0);
				while (auxV != NULL)
				{
					char temp[100];
					strcpy_s(temp, to_string(auxV->id).c_str());
					SendMessage(GetDlgItem(hwnd, LISTAVUELOS), LB_ADDSTRING, 0, (LPARAM)temp);
					auxV = auxV->sig;
				}
				SendMessage(GetDlgItem(hwnd, PILOTO), LB_RESETCONTENT, 0, 0);
				i = 0;
				auxP = inicioPilo;
				while (auxP != NULL)
				{
					if (auxP->status == true)
					{
						SendMessage(GetDlgItem(hwnd, PILOTO), CB_INSERTSTRING, (WPARAM)i, (LPARAM)auxP->nombre);
						i++;
					}
					auxP = auxP->sig;
				}


				EnableWindow(GetDlgItem(hwnd, REGISTRAR), true);
				EnableWindow(GetDlgItem(hwnd, ORIGEN), true);
				EnableWindow(GetDlgItem(hwnd, DESTINO), true);
				EnableWindow(GetDlgItem(hwnd, FECHAVUELO), true);
				EnableWindow(GetDlgItem(hwnd, FECHAREGISTRO), false);
				EnableWindow(GetDlgItem(hwnd, PILOTO), true);
				EnableWindow(GetDlgItem(hwnd, RANCHO), true);
				EnableWindow(GetDlgItem(hwnd, RESTRECHO), true);
				EnableWindow(GetDlgItem(hwnd, ASIENTOS), true);
				EnableWindow(GetDlgItem(hwnd, NINIOS), false);
				EnableWindow(GetDlgItem(hwnd, ADULTOS), false);
				EnableWindow(GetDlgItem(hwnd, CANCELAR), false);
				EnableWindow(GetDlgItem(hwnd, ABORDAR), false);
				SetDlgItemText(hwnd, ORIGEN, 0);
				SetDlgItemText(hwnd, DESTINO, 0);
				SetDlgItemText(hwnd, PILOTO, 0);
				SetDlgItemText(hwnd, ASIENTOS, 0);
				SetDlgItemText(hwnd, NINIOS, 0);
				SetDlgItemText(hwnd, ADULTOS, 0);
				SetDlgItemText(hwnd, USUREGISTRO, 0);
				SendDlgItemMessage(hwnd, RESTRECHO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
				SendDlgItemMessage(hwnd, RANCHO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
				SetDlgItemText(hwnd, ESTATUS, 0);

				//guardarVuelo();
				break;
			}
			else
			{
				MessageBox(NULL, "Registro incorrecto", "VUELOS", MB_OK);
				break;
			}
			break;
		}
		case CANCELAR:
			auxPa = inicioPa;
			auxB = inicioB;
			h = 0;
			while (auxPa != NULL) {
				if (h == 100) {
					auxPa = auxPa->sig;
					h = 0;
				}
				else if (auxPa->iDVuelo[h] == auxV->id) {
					auxPa->status = 1;
					h++;
				}
				else
					h++;
			}
			while (auxB != NULL) {
				if (auxB->iDVuelo == auxV->id) {
					auxB->status = 3;
					auxB = auxB->sig;
				}
				else
					auxB = auxB->sig;
			}
			auxV->estatus = 2;
			mostrarPiloto(inicioPilo, auxV->piloto);
			auxP->status = true;
			auxV = inicioV;
			SendMessage(GetDlgItem(hwnd, LISTAVUELOS), LB_RESETCONTENT, 0, 0);
			while (auxV != NULL) {
				char temp[100];
				strcpy_s(temp, to_string(auxV->id).c_str());
				SendMessage(GetDlgItem(hwnd, LISTAVUELOS), LB_ADDSTRING, 0, (LPARAM)temp);
				auxV = auxV->sig;
			}
			SendMessage(GetDlgItem(hwnd, PILOTO), LB_RESETCONTENT, 0, 0);
			i = 0;
			auxP = inicioPilo;
			while (auxP != NULL) {
				if (auxP->status == true) {
					SendMessage(GetDlgItem(hwnd, PILOTO), CB_INSERTSTRING, (WPARAM)i, (LPARAM)auxP->nombre);
					i++;
				}
				auxP = auxP->sig;
			}

			EnableWindow(GetDlgItem(hwnd, REGISTRAR), true);
			EnableWindow(GetDlgItem(hwnd, ORIGEN), true);
			EnableWindow(GetDlgItem(hwnd, DESTINO), true);
			EnableWindow(GetDlgItem(hwnd, FECHAVUELO), true);
			EnableWindow(GetDlgItem(hwnd, FECHAREGISTRO), false);
			EnableWindow(GetDlgItem(hwnd, PILOTO), true);
			EnableWindow(GetDlgItem(hwnd, RANCHO), true);
			EnableWindow(GetDlgItem(hwnd, RESTRECHO), true);
			EnableWindow(GetDlgItem(hwnd, ASIENTOS), true);
			EnableWindow(GetDlgItem(hwnd, NINIOS), false);
			EnableWindow(GetDlgItem(hwnd, ADULTOS), false);
			EnableWindow(GetDlgItem(hwnd, CANCELAR), false);
			EnableWindow(GetDlgItem(hwnd, ABORDAR), false);
			SetDlgItemText(hwnd, ORIGEN, 0);
			SetDlgItemText(hwnd, DESTINO, 0);
			SetDlgItemText(hwnd, PILOTO, 0);
			SetDlgItemText(hwnd, ASIENTOS, 0);
			SetDlgItemText(hwnd, NINIOS, 0);
			SetDlgItemText(hwnd, ADULTOS, 0);
			SetDlgItemText(hwnd, USUREGISTRO, 0);
			SendDlgItemMessage(hwnd, RESTRECHO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, RANCHO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			SetDlgItemText(hwnd, ESTATUS, 0);
			break;
		case ABORDAR:
			auxV->estatus = 1;
			mostrarPiloto(inicioPilo, auxV->piloto);
			auxP->status = true;
			i = 0;
			auxPa = inicioPa;
			auxB = inicioB;
			while (auxPa != NULL) {
				if (h == 100) {
					auxPa = auxPa->sig;
					h = 0;
				}
				else if (auxPa->iDVuelo[h] == auxV->id) {
					if (auxPa->status == 4)
						auxPa->status = 3;
					h++;
				}
				else
					h++;
			}
			while (auxB != NULL) {
				if (auxB->iDVuelo == auxV->id) {
					if (auxB->status == 0)
						auxB->status = 2;
					auxB = auxB->sig;
				}
				else
					auxB = auxB->sig;
			}

			//resetea
			auxV = inicioV;
			SendMessage(GetDlgItem(hwnd, LISTAVUELOS), LB_RESETCONTENT, 0, 0);
			while (auxV != NULL) {
				char temp[100];
				strcpy_s(temp, to_string(auxV->id).c_str());
				SendMessage(GetDlgItem(hwnd, LISTAVUELOS), LB_ADDSTRING, 0, (LPARAM)temp);
				auxV = auxV->sig;
			}
			SendMessage(GetDlgItem(hwnd, PILOTO), LB_RESETCONTENT, 0, 0);


			auxP = inicioPilo;
			while (auxP != NULL) {
				if (auxP->status != 4) {
					SendMessage(GetDlgItem(hwnd, PILOTO), CB_INSERTSTRING, (WPARAM)i, (LPARAM)auxP->nombre);
					i++;
				}
				auxP = auxP->sig;
			}
			EnableWindow(GetDlgItem(hwnd, REGISTRAR), true);
			EnableWindow(GetDlgItem(hwnd, ORIGEN), true);
			EnableWindow(GetDlgItem(hwnd, DESTINO), true);
			EnableWindow(GetDlgItem(hwnd, FECHAVUELO), true);
			EnableWindow(GetDlgItem(hwnd, FECHAREGISTRO), false);
			EnableWindow(GetDlgItem(hwnd, PILOTO), true);
			EnableWindow(GetDlgItem(hwnd, RANCHO), true);
			EnableWindow(GetDlgItem(hwnd, RESTRECHO), true);
			EnableWindow(GetDlgItem(hwnd, ASIENTOS), true);
			EnableWindow(GetDlgItem(hwnd, NINIOS), false);
			EnableWindow(GetDlgItem(hwnd, ADULTOS), false);
			EnableWindow(GetDlgItem(hwnd, CANCELAR), false);
			EnableWindow(GetDlgItem(hwnd, ABORDAR), false);
			SetDlgItemText(hwnd, ORIGEN, 0);
			SetDlgItemText(hwnd, DESTINO, 0);
			SetDlgItemText(hwnd, PILOTO, 0);
			SetDlgItemText(hwnd, ASIENTOS, 0);
			SetDlgItemText(hwnd, NINIOS, 0);
			SetDlgItemText(hwnd, ADULTOS, 0);
			SetDlgItemText(hwnd, USUREGISTRO, 0);
			SendDlgItemMessage(hwnd, RESTRECHO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, RANCHO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			SetDlgItemText(hwnd, ESTATUS, 0);
			break;
		}
		break;
	}
	return 0;
}

//FALTA ASIGNACION DE ASIENTO Y VUELO
//Falta que se guarde
LRESULT CALLBACK Pasajero(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HBRUSH pincel;
	switch (msg)
	{
	case WM_CLOSE:

		EndDialog(hwnd, LOWORD(wParam));
		break;
	case WM_CTLCOLORDLG:
		return (LRESULT)pincel;
	case WM_INITDIALOG:
		pincel = CreateSolidBrush(RGB(5, 105, 255));
		//mandar imagen
		hpControl = GetDlgItem(hwnd, FOTOUSUARIO2);
		HBITMAP bmp;
		bmp = (HBITMAP)LoadImage(NULL, actual->foto, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
		SendMessage(hpControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);

		SetDlgItemText(hwnd, IDC_EDIT1, actual->usuario);
		SetDlgItemText(hwnd, USUARIO, actual->usuario);
		SendMessage(GetDlgItem(hwnd, LISTAPASA), LB_RESETCONTENT, 0, 0);
		auxPa = inicioPa;
		while (auxPa != NULL)
		{
			if (auxPa->status != 5)
				SendMessage(GetDlgItem(hwnd, LISTAPASA), LB_ADDSTRING, 0, (LPARAM)auxPa->nombre);
			auxPa = auxPa->sig;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case REGISTRARPASA:
			pivotePa = new Pasajeros();
			GetDlgItemText(hwnd, NOMBRE, pivotePa->nombre, 100);

			GetDlgItemText(hwnd, APELLIDOP, pivotePa->apellidoP, 100);
			GetDlgItemText(hwnd, APELLIDOM, pivotePa->apellidoM, 100);

			//Fechas
			SYSTEMTIME systemTime;
			SendDlgItemMessage(hwnd, NACIMIENTO, DTM_GETSYSTEMTIME, 0, (LPARAM)&systemTime);
			snprintf(pivotePa->nacimiento, sizeof(pivotePa->nacimiento), "%02d/%02d/%04d %02d:%02d:%02d",
				systemTime.wMonth, systemTime.wDay, systemTime.wYear,
				systemTime.wHour, systemTime.wMinute, systemTime.wSecond);

			GetSystemTime(&systemTime);
			snprintf(pivotePa->registro, sizeof(pivotePa->registro), "%02d/%02d/%04d %02d:%02d:%02d",
				systemTime.wMonth, systemTime.wDay, systemTime.wYear,
				systemTime.wHour, systemTime.wMinute, systemTime.wSecond);

			GetDlgItemText(hwnd, USUARIO, pivotePa->usuRegistro, 100);

			if (IsDlgButtonChecked(hwnd, RHOMBRE) == BST_CHECKED)
				pivotePa->genero = 1;
			if (IsDlgButtonChecked(hwnd, RMUJER) == BST_CHECKED)
				pivotePa->genero = 0;

			GetDlgItemText(hwnd, NACIONALIDAD, pivotePa->nacionalidad, 100);
			pasId++;

			pivotePa->id = pasId;
			pivotePa->status = 0;

			agregarPasajero(inicioPa, pivotePa);
			guardarPasajero();

			SendMessage(GetDlgItem(hwnd, LISTAPASA), LB_RESETCONTENT, 0, 0);
			auxPa = inicioPa;

			while (auxPa != NULL)
			{
				if (auxPa->status != 5)
					SendMessage(GetDlgItem(hwnd, LISTAPASA), LB_ADDSTRING, 0, (LPARAM)auxPa->nombre);
				auxPa = auxPa->sig;
			}

			SetDlgItemText(hwnd, NACIONALIDAD, 0);
			SetDlgItemText(hwnd, NOMBRE, 0);
			SetDlgItemText(hwnd, APELLIDOP, 0);
			SetDlgItemText(hwnd, APELLIDOM, 0);
			SetDlgItemText(hwnd, asientoID, 0);
			SetDlgItemText(hwnd, vueloID, 0);
			SendDlgItemMessage(hwnd, RMUJER, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, RHOMBRE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

			while (auxP != NULL)
			{
				SendMessage(GetDlgItem(hwnd, LISTAPILO), LB_ADDSTRING, 0, (LPARAM)auxP->nombre);
				auxP = auxP->sig;
			}
			//

			break;
		case MODIFICARPASA:
			pivotePa = new Pasajeros();
			GetDlgItemText(hwnd, NOMBRE, pivotePa->nombre, 100);
			GetDlgItemText(hwnd, APELLIDOP, pivotePa->apellidoP, 100);
			GetDlgItemText(hwnd, APELLIDOM, pivotePa->apellidoM, 100);
			GetDlgItemText(hwnd, NACIONALIDAD, pivotePa->nacionalidad, 100);
			modificarPasajero(auxPa, pivotePa->nombre, pivotePa->apellidoP, pivotePa->apellidoM, pivotePa->nacionalidad);
			/*guardarPasajero();*/
			SetDlgItemText(hwnd, NACIONALIDAD, 0);
			SetDlgItemText(hwnd, NOMBRE, 0);
			SetDlgItemText(hwnd, APELLIDOP, 0);
			SetDlgItemText(hwnd, APELLIDOM, 0);
			SendDlgItemMessage(hwnd, RMUJER, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, RHOMBRE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			SendMessage(GetDlgItem(hwnd, LISTAPASA), LB_RESETCONTENT, 0, 0);
			auxPa = inicioPa;
			while (auxPa != NULL)
			{
				if (auxPa->status != 5)
					SendMessage(GetDlgItem(hwnd, LISTAPASA), LB_ADDSTRING, 0, (LPARAM)auxPa->nombre);
				auxPa = auxPa->sig;
			}
			break;

		case ELIMINARPASA:
			//si elimino al pasajero y tiene un vuelo, automaticamente se le pone "No abordado"
			if (auxPa->status == 4) {
				h = 0;
				while (auxPa->claveAsiento[h + 1] != 0 && h != 100)
					h++;
				auxB = inicioB;
				while (auxB != NULL) {
					if (auxB->claveAsiento == auxPa->claveAsiento[h]) {
						auxB->status = 2;	//Asignado que no abordo
						/*
						mostrarVuelo(auxB->iDVuelo);
						auxV->asientos++;
						if (auxB->generacion == 0)
							auxV->childPercent++;
						if (auxB->generacion == 2)
							auxV->oldPercent++;*/
					}
					auxB = auxB->sig;
				}
			}

			eliminarPasajero(auxPa);
			SendMessage(GetDlgItem(hwnd, LISTAPASA), LB_RESETCONTENT, 0, 0);
			auxPa = inicioPa;
			while (auxPa != NULL)
			{
				if (auxPa->status != 5)
					SendMessage(GetDlgItem(hwnd, LISTAPASA), LB_ADDSTRING, 0, (LPARAM)auxPa->nombre);
				auxPa = auxPa->sig;
			}
			break;

		case LISTAPASA:
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
				SendDlgItemMessage(hwnd, RHOMBRE, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
				SendDlgItemMessage(hwnd, RMUJER, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
				EnableWindow(GetDlgItem(hwnd, NACIMIENTO), false);
				EnableWindow(GetDlgItem(hwnd, RHOMBRE), false);
				EnableWindow(GetDlgItem(hwnd, RMUJER), false);
				EnableWindow(GetDlgItem(hwnd, MODIFICARPASA), true);
				EnableWindow(GetDlgItem(hwnd, REGISTRARPASA), false);
				EnableWindow(GetDlgItem(hwnd, ELIMINARPASA), true);
				int indice;
				indice = SendDlgItemMessage(hwnd, LISTAPASA, LB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hwnd, LISTAPASA, LB_GETTEXT, indice, (LPARAM)usuario);
				auxPa = inicioPa;
				while (auxPa != NULL) {
					if (strcmp(auxPa->nombre, usuario) == 0)
						break;
					else
						auxPa = auxPa->sig;
				}

				SetDlgItemText(hwnd, NOMBRE, auxPa->nombre);
				SetDlgItemText(hwnd, APELLIDOP, auxPa->apellidoP);
				SetDlgItemText(hwnd, APELLIDOM, auxPa->apellidoM);

				if (auxPa->genero == true)
					SendDlgItemMessage(hwnd, RHOMBRE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				else
					SendDlgItemMessage(hwnd, RMUJER, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

				SYSTEMTIME st;
				st = convertirFecha(auxPa->nacimiento);
				SendDlgItemMessage(hwnd, NACIMIENTO, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&st);
				st = convertirFecha(auxPa->registro);
				SendDlgItemMessage(hwnd, REGISTRO, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&st);

				SetDlgItemText(hwnd, USUREGISTRO, auxPa->usuRegistro);
				int b = 0;
				if (auxPa->status == 0) {
					SetDlgItemText(hwnd, asientoID, "No tiene boleto");
					SetDlgItemText(hwnd, vueloID, "No tiene vuelo");
				}
				else {
					while (auxPa->claveAsiento[b + 1] != 0 && b != 100)
						b++;
					SetDlgItemInt(hwnd, asientoID, auxPa->claveAsiento[b], 0);
					b = 0;
					while (auxPa->iDVuelo[b + 1] != 0 && b != 100)
						b++;
					SetDlgItemInt(hwnd, vueloID, auxPa->iDVuelo[b], 0);
				}
				SetDlgItemText(hwnd, NACIONALIDAD, auxPa->nacionalidad);
				break;
			}
		}
		break;
	}
	return 0;
}

LRESULT CALLBACK Compra(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i = 0;
	static HBRUSH pincel;
	switch (msg)
	{
	case WM_CLOSE:
		EndDialog(hwnd, LOWORD(wParam));
		break;
	case WM_CTLCOLORDLG:
		return (LRESULT)pincel;

	case WM_INITDIALOG:
		pincel = CreateSolidBrush(RGB(5, 105, 255));
		//mandar imagen
		hpControl = GetDlgItem(hwnd, FOTOUSUARIO);
		HBITMAP bmp;
		bmp = (HBITMAP)LoadImage(NULL, actual->foto, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
		SendMessage(hpControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);
		/*SetDlgItemText(hwnd, IDC_EDIT1, actual->usuario);*/
		SetDlgItemText(hwnd, ADMIN, actual->usuario);
		auxV = inicioV;
		SendMessage(GetDlgItem(hwnd, LISTAVUELOS), LB_RESETCONTENT, 0, 0);
		while (auxV != NULL) {
			char temp[100];
			strcpy_s(temp, to_string(auxV->id).c_str());
			if (auxV->estatus == 0 && auxV->asientos != 0)
				SendMessage(GetDlgItem(hwnd, LISTAVUELOS), LB_ADDSTRING, 0, (LPARAM)temp);
			auxV = auxV->sig;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case LISTAVUELOS:
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				EnableWindow(GetDlgItem(hwnd, PASAJERO), true);
				EnableWindow(GetDlgItem(hwnd, RTURISTA), true);
				EnableWindow(GetDlgItem(hwnd, REJECUTIVO), true);
				EnableWindow(GetDlgItem(hwnd, GENERACION), true);
				EnableWindow(GetDlgItem(hwnd, PAGO), true);

				auxPa = inicioPa;
				SendMessage(GetDlgItem(hwnd, PASAJERO), CB_RESETCONTENT, 0, 0);
				while (auxPa != NULL) {
					if (auxPa->status != 4) {
						SendMessage(GetDlgItem(hwnd, PASAJERO), CB_INSERTSTRING, (WPARAM)i, (LPARAM)auxPa->nombre);
						i++;
					}
					auxPa = auxPa->sig;
				}

				int indice;
				int idNow;
				indice = SendDlgItemMessage(hwnd, LISTAVUELOS, LB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hwnd, LISTAVUELOS, LB_GETTEXT, indice, (LPARAM)usuario);
				idNow = stoi(usuario);
				mostrarVuelo(idNow);
				SetDlgItemText(hwnd, ORIGEN, auxV->origen);
				SetDlgItemText(hwnd, DESTINO, auxV->destino);
				SetDlgItemInt(hwnd, ASIENTOS, auxV->asientos, 0);
				SetDlgItemInt(hwnd, MENORES, auxV->childPercent, 0);
				SetDlgItemInt(hwnd, MAYORES, auxV->oldPercent, 0);

				SendMessage(GetDlgItem(hwnd, GENERACION), CB_RESETCONTENT, 0, 0);
				i = 0;
				if (auxV->oldPercent != 0) {
					SendMessage(GetDlgItem(hwnd, GENERACION), CB_INSERTSTRING, (WPARAM)i, (LPARAM)"Mayor de edad");
					i++;
				}
				if (auxV->childPercent != 0) {
					SendMessage(GetDlgItem(hwnd, GENERACION), CB_INSERTSTRING, (WPARAM)i, (LPARAM)"Menor de edad");
					i++;
				}
				SendMessage(GetDlgItem(hwnd, GENERACION), CB_INSERTSTRING, (WPARAM)i, (LPARAM)"Adulto");
				SendMessage(GetDlgItem(hwnd, PAGO), CB_RESETCONTENT, 0, 0);
				i = 0;
				SendMessage(GetDlgItem(hwnd, PAGO), CB_INSERTSTRING, (WPARAM)i, (LPARAM)"Credito");
				i++;
				SendMessage(GetDlgItem(hwnd, PAGO), CB_INSERTSTRING, (WPARAM)i, (LPARAM)"Debito");
				i++;
				SendMessage(GetDlgItem(hwnd, PAGO), CB_INSERTSTRING, (WPARAM)i, (LPARAM)"Efectivo");
				break;
			}
			break;
		case COMPRAR:
			//PASAJERO
			pivotePa = new Pasajeros();
			SendDlgItemMessage(hwnd, PASAJERO, WM_GETTEXT, 100, (LPARAM)pivotePa->nombre);
			auxPa = inicioPa;
			while (auxPa != NULL) {
				if (strcmp(auxPa->nombre, pivotePa->nombre) == 0)
					break;
				else
					auxPa = auxPa->sig;
			}

			auxPa->status = 4;	//Compro boleto
			h = 0;
			while (auxPa->iDVuelo[h] != 0 && h != 100)
				h++;
			auxPa->iDVuelo[h] = auxV->id;
			pivoteB = new Boleto();
			pivoteB->iDVuelo = auxV->id;
			if (IsDlgButtonChecked(hwnd, RTURISTA) == BST_CHECKED)
				pivoteB->tipo = 0;
			if (IsDlgButtonChecked(hwnd, REJECUTIVO) == BST_CHECKED)
				pivoteB->tipo = 1;

			SendDlgItemMessage(hwnd, GENERACION, WM_GETTEXT, 100, (LPARAM)usuario);
			if (strcmp(usuario, "Menor de edad") == 0) {
				pivoteB->generacion = 1;
				auxV->childPercent--;
			}
			if (strcmp(usuario, "Adulto") == 0)
				pivoteB->generacion = 1;
			if (strcmp(usuario, "Mayor de edad") == 0) {
				pivoteB->generacion = 2;
				auxV->oldPercent--;
			}
			auxV->asientos--;

			SendDlgItemMessage(hwnd, PAGO, WM_GETTEXT, 100, (LPARAM)usuario);
			if (strcmp(usuario, "Credito") == 0)
				pivoteB->pago = 0;
			if (strcmp(usuario, "Debito") == 0)
				pivoteB->pago = 1;
			if (strcmp(usuario, "Efectivo") == 0)
				pivoteB->pago = 2;

			pivoteB->claveAsiento = ++boletoId;
			pivoteB->status = 0;
			agregarBoleto(inicioB, pivoteB);

			auxPa->claveAsiento[h] = pivoteB->claveAsiento;
			auxPa = inicioPa;
			SendMessage(GetDlgItem(hwnd, PASAJERO), CB_RESETCONTENT, 0, 0);
			while (auxPa != NULL) {
				if (auxPa->status == 0) {
					SendMessage(GetDlgItem(hwnd, PASAJERO), CB_INSERTSTRING, (WPARAM)i, (LPARAM)auxPa->nombre);
					i++;
				}
				auxPa = auxPa->sig;
			}
			auxV = inicioV;
			SendMessage(GetDlgItem(hwnd, LISTAVUELOS), LB_RESETCONTENT, 0, 0);
			while (auxV != NULL) {
				char temp[100];
				strcpy_s(temp, to_string(auxV->id).c_str());
				if (auxV->estatus == 0 && auxV->asientos != 0)
					SendMessage(GetDlgItem(hwnd, LISTAVUELOS), LB_ADDSTRING, 0, (LPARAM)temp);
				auxV = auxV->sig;
			}
			SendDlgItemMessage(hwnd, RTURISTA, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
			SendDlgItemMessage(hwnd, REJECUTIVO, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);

			break;
		}
	}
	return 0;
}

void llenarCancelar(HWND hwnd) {
	//hay que validar que si la fecha es menor a 10 dias, no muestre el boleto para cancelacion, de ahí usamos las
	//funciones que te habia pedido del tiempo, por mientras solo con que los muestre
	auxB = inicioB;
	SendMessage(GetDlgItem(hwnd, LISTABOLETOS), LB_RESETCONTENT, 0, 0);
	while (auxB != NULL) {
		//valida que sea un boleto COMPRADO y no en otro estado
		if (auxB->status == 0) {
			char temp[10];
			strcpy_s(temp, to_string(auxB->claveAsiento).c_str());
			SendMessage(GetDlgItem(hwnd, LISTABOLETOS), LB_ADDSTRING, 0, (LPARAM)temp);
		}
		auxB = auxB->sig;
	}
}

LRESULT CALLBACK CancelarCompra(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HBRUSH pincel;
	switch (msg)

	{
	case WM_CLOSE:
		EndDialog(hwnd, LOWORD(wParam));
		break;
	case WM_CTLCOLORDLG:
		return (LRESULT)pincel;
	case WM_INITDIALOG:
		pincel = CreateSolidBrush(RGB(5, 105, 255));
		llenarCancelar(hwnd);
		//mandar imagen
		hpControl = GetDlgItem(hwnd, FOTOUSUARIO);
		HBITMAP bmp;
		bmp = (HBITMAP)LoadImage(NULL, actual->foto, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
		SendMessage(hpControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);

		SetDlgItemText(hwnd, IDC_EDIT1, actual->usuario);
		SetDlgItemText(hwnd, ADMIN, actual->usuario);

		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case CANCELAR:
			auxPa->status = 1;
			auxB->status = 3;
			auxV->asientos++;
			if (auxB->generacion == 0)
				auxV->childPercent++;
			if (auxB->generacion == 2)
				auxV->oldPercent++;
			llenarCancelar(hwnd);
			EnableWindow(GetDlgItem(hwnd, CANCELAR), false);
			SetDlgItemText(hwnd, PASAJERO, 0);
			SetDlgItemText(hwnd, PATERNO, 0);  
			SetDlgItemText(hwnd, MATERNO, 0); 
			SetDlgItemText(hwnd, VUELO, 0);
			SetDlgItemText(hwnd, ASIENTO, 0);
			SetDlgItemText(hwnd, ORIGEN, 0);
			SetDlgItemText(hwnd, DESTINO, 0);
			break;
		case LISTABOLETOS:
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				EnableWindow(GetDlgItem(hwnd, CANCELAR), true);
				int indice = SendDlgItemMessage(hwnd, LISTABOLETOS, LB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hwnd, LISTABOLETOS, LB_GETTEXT, indice, (LPARAM)usuario);
				auxPa = inicioPa;
				int p = stoi(usuario);
				h = 0;
				while (auxPa != NULL && auxPa->claveAsiento[h] != p) {
					if (h == 10) {
						h = 0;
						auxPa = auxPa->sig;
					}
					else
						h++;
				}
				auxB = inicioB;
				while (auxB != NULL && auxB->claveAsiento != p)
					auxB = auxB->sig;
				auxV = inicioV;
				while (auxV != NULL && auxV->id != auxB->iDVuelo)
					auxV = auxV->sig;
				SetDlgItemText(hwnd, PASAJERO, auxPa->nombre);
				SetDlgItemText(hwnd, PATERNO, auxPa->apellidoP);
				SetDlgItemText(hwnd, MATERNO, auxPa->apellidoM);
				mostrarVuelo(auxPa->iDVuelo[h]);
				SetDlgItemInt(hwnd, VUELO, auxPa->iDVuelo[h], 0);
				SetDlgItemInt(hwnd, ASIENTO, auxPa->claveAsiento[h], 0);
				SetDlgItemText(hwnd, ORIGEN, auxV->origen);
				SetDlgItemText(hwnd, DESTINO, auxV->destino);
				break;
			}
			break;
		}
		break;
	}
	return 0;
}

LRESULT CALLBACK Abordaje(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HBRUSH pincel;
	switch (msg)
	{
	case WM_CLOSE:
		EndDialog(hwnd, LOWORD(wParam));
		break;
	case WM_CTLCOLORDLG:
		return (LRESULT)pincel;
	case WM_INITDIALOG:
		pincel = CreateSolidBrush(RGB(5, 105, 255));
		//mandar imagen
		hpControl = GetDlgItem(hwnd, FOTOUSUARIO);
		HBITMAP bmp;
		bmp = (HBITMAP)LoadImage(NULL, actual->foto, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
		SendMessage(hpControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);

		SetDlgItemText(hwnd, IDC_EDIT1, actual->usuario);
		SetDlgItemText(hwnd, ADMIN, actual->usuario);
		auxB = inicioB;
		SendMessage(GetDlgItem(hwnd, LISTABOLETOS), LB_RESETCONTENT, 0, 0);
		while (auxB != NULL) {
			if (auxB->status != 3) {
				char temp[10];
				strcpy_s(temp, to_string(auxB->claveAsiento).c_str());
				SendMessage(GetDlgItem(hwnd, LISTABOLETOS), LB_ADDSTRING, 0, (WPARAM)temp);
			}
			auxB = auxB->sig;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ABORDO:
			auxB->status = 1;
			auxPa->status = 2;
			SetDlgItemText(hwnd, PASAJERO, 0);
			SetDlgItemText(hwnd, PATERNO, 0);
			SetDlgItemText(hwnd, MATERNO, 0);
			SetDlgItemText(hwnd, VUELO, 0);
			SetDlgItemText(hwnd, ASIENTO, 0);
			SetDlgItemText(hwnd, ORIGEN, 0);
			SetDlgItemText(hwnd, DESTINO, 0);
			SetDlgItemText(hwnd, ESTADO, 0);
			EnableWindow(GetDlgItem(hwnd, ABORDO), false);
			EnableWindow(GetDlgItem(hwnd, NOABORDO), false);
			break;
		case NOABORDO:
			auxB->status = 2;
			auxPa->status = 3;
			break;
		case LISTABOLETOS:
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				int indice = SendDlgItemMessage(hwnd, LISTABOLETOS, LB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hwnd, LISTABOLETOS, LB_GETTEXT, indice, (LPARAM)usuario);
				int p = stoi(usuario);
				auxB = inicioB;
				while (auxB != NULL && auxB->claveAsiento != p)
					auxB = auxB->sig;
				auxV = inicioV;
				while (auxV != NULL && auxV->id != auxB->iDVuelo)
					auxV = auxV->sig;
				auxPa = inicioPa;
				h = 0;
				while (auxPa->claveAsiento[h] != p) {
					if (h == 10) {
						h = 0;
						auxPa = auxPa->sig;
					}
					else
						h++;
				}
				SetDlgItemText(hwnd, PASAJERO, auxPa->nombre);
				SetDlgItemText(hwnd, PATERNO, auxPa->apellidoP);
				SetDlgItemText(hwnd, MATERNO, auxPa->apellidoM);
				SetDlgItemInt(hwnd, VUELO, auxV->id, 0);
				SetDlgItemInt(hwnd, ASIENTO, auxB->claveAsiento, 0);
				SetDlgItemText(hwnd, ORIGEN, auxV->origen);
				SetDlgItemText(hwnd, DESTINO, auxV->destino);
				//aun no aborda
				if (auxB->status == 0)
					SetDlgItemText(hwnd, ESTADO, "Aun no aborda");
				if (auxB->status == 1)
					SetDlgItemText(hwnd, ESTADO, "Abordo");
				if (auxB->status == 2)
					SetDlgItemText(hwnd, ESTADO, "No abordo");
				EnableWindow(GetDlgItem(hwnd, ABORDO), true);
				EnableWindow(GetDlgItem(hwnd, NOABORDO), true);
				break;
			}
			break;

		}
		break;
	}
	return 0;
}

/*
-----------------MANIFIESTO
Será agarrar la informacion del boleto, vuelo y pasajero para mostrarla en la pantalla, la forma de guardado seria:
\t\t\tVUELO ID\t\tORIGEN\t\tDESTINO
PASAJERO NOMBRE COMPLETO\t\tASIENTO\t\tEDAD\t\tPAGO\t\tTIPO
*/

void generarManifiesto() {
	string linea;
	linea = to_string(auxV->id);
	linea = linea + "-";
	linea = linea + auxV->origen;
	linea = linea + "-";
	linea = linea + auxV->destino;
	ofstream archivo(linea + ".txt");
	auxPa = inicioPa;
	auxB = inicioB;
	archivo << "ID del Vuelo: " << auxV->id << "\t\t Origen: " << auxV->origen << "\t Destino: " << auxV->destino << "\tFecha: " << auxV->fecha << endl;
	h = 0;
	while (auxB != NULL) {
		if (auxB->status != 3) {
			if (auxB->iDVuelo == auxV->id) {
				while (h != 100 && auxPa->claveAsiento[h] != auxB->claveAsiento)
					h++;
				if (h == 100) {
					h = 0;
					auxPa = auxPa->sig;
				}
				else {
					archivo << auxPa->nombre << " " << auxPa->apellidoP << " " << auxPa->apellidoM << " Edad:" <<
						auxPa->edad << " Asiento: " << auxPa->claveAsiento[h] << " Nacionalidad: " << auxPa->nacionalidad <<
						" Estatus: ";
					if (auxB->status == 0)
						archivo << "Compro el boleto" << endl;
					if (auxB->status == 1)
						archivo << "Abordo" << endl;
					if (auxB->status == 2)
						archivo << "No abordo" << endl;
					h = 0;
					auxPa = inicioPa;
					auxB = auxB->sig;
				}
			}
			else {
				auxB = auxB->sig;
			}
		}
		else {
			auxB = auxB->sig;
		}
	}
	archivo.close();
}

LRESULT CALLBACK Manifiesto(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HBRUSH pincel;
	switch (msg)
	{
	case WM_CLOSE:
		EndDialog(hwnd, LOWORD(wParam));
		break;
	case WM_CTLCOLORDLG:
		return (LRESULT)pincel;
	case WM_INITDIALOG:
		pincel = CreateSolidBrush(RGB(5, 105, 255));
		//mandar imagen
		hpControl = GetDlgItem(hwnd, FOTOUSUARIO);
		HBITMAP bmp;
		bmp = (HBITMAP)LoadImage(NULL, actual->foto, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
		SendMessage(hpControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)bmp);

		SetDlgItemText(hwnd, IDC_EDIT1, actual->usuario);
		SetDlgItemText(hwnd, ADMIN, actual->usuario);
		auxV = inicioV;
		SendMessage(GetDlgItem(hwnd, LISTAVUELOS), LB_RESETCONTENT, 0, 0);
		while (auxV != NULL) {
			char temp[100];
			strcpy_s(temp, to_string(auxV->id).c_str());
			SendMessage(GetDlgItem(hwnd, LISTAVUELOS), LB_ADDSTRING, 0, (LPARAM)temp);
			auxV = auxV->sig;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ARCHIVAR:
			//ACA PONES PARA GUARDARLO EN UN TXT
			generarManifiesto();
			SYSTEMTIME st;
			GetSystemTime(&st);
			SetDlgItemText(hwnd, ORIGEN, 0);
			SetDlgItemText(hwnd, DESTINO, 0);
			SetDlgItemText(hwnd, ASIENTOS, 0);
			SetDlgItemText(hwnd, MENOR, 0);
			SetDlgItemText(hwnd, MAYOR, 0);
			SendDlgItemMessage(hwnd, FECHA, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&st);
			SetDlgItemText(hwnd, NOMBRE, 0);
			SetDlgItemText(hwnd, PATERNO, 0);
			SetDlgItemText(hwnd, MATERNO, 0);
			SetDlgItemText(hwnd, EDAD, 0);
			SetDlgItemText(hwnd, NACION, 0);
			SendMessage(GetDlgItem(hwnd, LISTA2), LB_RESETCONTENT, 0, 0);
			break;

		case ASIENTO:
			SendMessage(GetDlgItem(hwnd, LISTA2), LB_RESETCONTENT, 0, 0);
			//manda todos los boletos por numero asiento
			auxB = inicioB;
			while (auxB != NULL) {
				while (auxB != NULL && auxB->iDVuelo != auxV->id)
					auxB = auxB->sig;
				if (auxB != NULL) {
					if (auxB->status != 3) {
						char temp[100];
						strcpy_s(temp, to_string(auxB->claveAsiento).c_str());
						SendMessage(GetDlgItem(hwnd, LISTA2), LB_ADDSTRING, 0, (LPARAM)temp);
					}
					auxB = auxB->sig;
				}
			}
			break;
		case APELLIDO:
			SendMessage(GetDlgItem(hwnd, LISTA2), LB_RESETCONTENT, 0, 0);
			auxPa = inicioPa;
			while (auxPa != NULL) {
				auxB = inicioB;
				while (auxB != NULL) {
					h = 0;
					if (auxB->iDVuelo == auxV->id) {
						while (h != 100) {
							while (h != 100 && auxPa->claveAsiento[h] != auxB->claveAsiento)
								h++;
							//encontro el asiento
							if (h != 100 && auxPa->claveAsiento[h] == auxB->claveAsiento) {
								if (auxB->status != 3)
									SendMessage(GetDlgItem(hwnd, LISTA2), LB_ADDSTRING, 0, (LPARAM)auxPa->apellidoP);
								h = 100;
							}
						}
					}
					auxB = auxB->sig;
				}
				auxPa = auxPa->sig;
			}
			break;
		case LISTAVUELOS:
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				EnableWindow(GetDlgItem(hwnd, ASIENTO), true);
				EnableWindow(GetDlgItem(hwnd, APELLIDO), true);

				int indice = SendDlgItemMessage(hwnd, LISTAVUELOS, LB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hwnd, LISTAVUELOS, LB_GETTEXT, indice, (LPARAM)usuario);
				int p = stoi(usuario);
				mostrarVuelo(p);
				//Aca tengo el auxV listo
				SetDlgItemText(hwnd, ORIGEN, auxV->origen);
				SetDlgItemText(hwnd, DESTINO, auxV->destino);
				SetDlgItemInt(hwnd, ASIENTOS, auxV->totalAsientos, 0);
				SetDlgItemInt(hwnd, MENOR, auxV->totalChild, 0);
				SetDlgItemInt(hwnd, MAYOR, auxV->totalOld, 0);
				SYSTEMTIME st;
				st = convertirFecha(auxV->fecha);
				SendDlgItemMessage(hwnd, FECHA, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&st);

				SendMessage(GetDlgItem(hwnd, LISTA2), LB_RESETCONTENT, 0, 0);
				//manda todos los boletos por numero asiento
				auxB = inicioB;
				while (auxB != NULL) {
					while (auxB != NULL && auxB->iDVuelo != auxV->id)
						auxB = auxB->sig;
					if (auxB != NULL) {
						if (auxB->status != 3) {
							char temp[100];
							strcpy_s(temp, to_string(auxB->claveAsiento).c_str());
							SendMessage(GetDlgItem(hwnd, LISTA2), LB_ADDSTRING, 0, (LPARAM)temp);
						}
						auxB = auxB->sig;
					}
				}
				break;
			}
			break;
		case LISTA2:
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				int indice = SendDlgItemMessage(hwnd, LISTA2, LB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hwnd, LISTA2, LB_GETTEXT, indice, (LPARAM)usuario);
				if (isdigit(usuario[0])) {
					//Agarro el boleto
					int u = stoi(usuario);
					auxPa = inicioPa;
					while (auxPa != NULL) {
						h = 0;
						while (auxPa != NULL && h != 100 && auxPa->claveAsiento[h] != u) {
							h++;
						}
						if (h != 100 && auxPa->claveAsiento[h] == u) {
							SetDlgItemText(hwnd, NOMBRE, auxPa->nombre);
							SetDlgItemText(hwnd, PATERNO, auxPa->apellidoP);
							SetDlgItemText(hwnd, MATERNO, auxPa->apellidoM);
							//Edad
							SYSTEMTIME hoy;
							GetSystemTime(&hoy);
							SYSTEMTIME cumple;
							cumple = convertirFecha(auxPa->nacimiento);
							int edad = hoy.wYear - cumple.wYear;
							if (hoy.wMonth < cumple.wMonth)
								edad--;
							if (hoy.wMonth == cumple.wMonth && hoy.wDay < cumple.wDay)
								edad--;
							SetDlgItemInt(hwnd, EDAD, edad, 0);
							auxPa->edad = edad;

							SetDlgItemText(hwnd, NACION, auxPa->nacionalidad);
							auxB = inicioB;
							while (auxB != NULL && auxB->claveAsiento != u) {
								auxB = auxB->sig;
							}
							if (auxB->status == 0)
								SetDlgItemText(hwnd, ESTADO, "Aun no aborda");
							if (auxB->status == 1)
								SetDlgItemText(hwnd, ESTADO, "Abordo");
							if (auxB->status == 2)
								SetDlgItemText(hwnd, ESTADO, "No Abordo");
							if (auxB->status == 3)
								SetDlgItemText(hwnd, ESTADO, "Cancelado");
						}
						auxPa = auxPa->sig;
					}
				}
				auxPa = inicioPa;
				//Apellido
				if (isalpha(usuario[0])) {
					while (auxPa != NULL) {
						auxB = inicioB;
						if (strcmp(usuario, auxPa->apellidoP) == 0) {
							while (auxB != NULL) {
								for (int t = 0; t < 100; t++) {
									if (auxPa->claveAsiento[t] == auxB->claveAsiento && auxB->status != 3) {
										SetDlgItemText(hwnd, NOMBRE, auxPa->nombre);
										SetDlgItemText(hwnd, PATERNO, auxPa->apellidoP);
										SetDlgItemText(hwnd, MATERNO, auxPa->apellidoM);
										//Edad
										SYSTEMTIME hoy;
										GetSystemTime(&hoy);
										SYSTEMTIME cumple;
										cumple = convertirFecha(auxPa->nacimiento);
										int edad = hoy.wYear - cumple.wYear;
										if (hoy.wMonth < cumple.wMonth)
											edad--;
										if (hoy.wMonth == cumple.wMonth && hoy.wDay < cumple.wDay)
											edad--;
										SetDlgItemInt(hwnd, EDAD, edad, 0);
										auxPa->edad = edad;
										SetDlgItemText(hwnd, NACION, auxPa->nacionalidad);

										if (auxB->status == 0)
											SetDlgItemText(hwnd, ESTADO, "Aun no aborda");
										if (auxB->status == 1)
											SetDlgItemText(hwnd, ESTADO, "Abordo");
										if (auxB->status == 2)
											SetDlgItemText(hwnd, ESTADO, "No Abordo");
										if (auxB->status == 3)
											SetDlgItemText(hwnd, ESTADO, "Cancelado");
										return 0;
									}
								}
								auxB = auxB->sig;
							}
						}
						else
							auxPa = auxPa->sig;
					}
				}


				break;
			}
			break;
		}
		break;
	}
	return 0;
}