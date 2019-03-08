#include <Windows.h>
#include <windowsx.h>
#include <fstream>
#include <ctime>
#include <mmsystem.h>
#include <Commctrl.h>
#include <commdlg.h>
#include "resource.h"
#pragma comment(lib, "winmm.lib")

#define TM_DATESTIMER 10

using namespace std;

struct contact
{
	char nombre[25]{};
	char apellidos[50]{};
	char direccion[35]{};
	bool genero;
	int relacion;
	char correo[50]{};
	char telefono[10]{};
	char cancion[MAX_PATH]{};
	char foto1[MAX_PATH]{};
	char foto2[MAX_PATH]{};
	contact *prev = NULL;
	contact *next = NULL;
}*firstCont = NULL, *lastCont = NULL, *newCont = NULL, auxCont, *auxCp = NULL;

struct date
{
	char dateName[75];
	char cntName[50];
	char cntAp[75];
	char relacion[20];
	char telefono[10];
	int alarmCursel = 0;
	char alarm[260];
	char comment[MAX_PATH];
	tm dateAndTime;
	bool attended = false;
	date *prev = NULL;
	date *next = NULL;
}*firstDate = NULL, *lastDate = NULL, *newDate = NULL, auxDate, *auxDatesTp = NULL, *auxCal = NULL;

_SYSTEMTIME dateDay, dateTime, currentTime;
//Arreglos de caracteres para cada uno de las direcciones de los archivos
TCHAR contsDirection[MAX_PATH + 1];
TCHAR datesDirection[MAX_PATH + 1];
TCHAR catsDirection[MAX_PATH + 1];
TCHAR alarm1Dir[MAX_PATH + 1];/* Vaquita */
TCHAR alarm2Dir[MAX_PATH + 1];/* Pop-up */
TCHAR alarm3Dir[MAX_PATH + 1];/* Campana */

UINT datesTimer = 0;
char dateName[80]{};
char bufferString[100]{};
char categoria[20]{};
char fullName[150];
char dateMessage[MAX_PATH]{};
char alarmString[100]{};
int pIndex = 0;
bool emptyField;

HBITMAP pic1, pic2, editPic1, editPic2;
//Form handlers
HINSTANCE hinst;
HWND wnHndl = 0;
HWND hAddCntsFRM = 0;
HWND hCntsFRM = 0;
HWND hAddDates = 0;
HWND hCalendar = 0;
HWND hMonth = 0;
HWND hEditDate = 0;

//Callback functions
BOOL CALLBACK mainFRM(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK addCont(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK conts(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK addDate(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK editDate(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK calendar(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
//Misc functions
bool SelectFile(HWND Dialog, int text, int Filter)/*1 = imagenes / 2 = audio*/ 
{
	LPCSTR filtros;
	if (Filter == 1)
		filtros = "Todos los archivos\0*.*\0Imagenes BMP\0*.bmp\0";
	else if (Filter == 2)
		filtros = "Todos los archivos\0*.*\0Canciones MP3\0*.mp3\0";
	char szFile[MAX_PATH];       // buffer for file name

	OPENFILENAME ofn;       // common dialog box structure	
	HANDLE hf;              // file handle
							// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = Dialog;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrFilter = filtros; // Variable según el tipo de filtro
	ofn.nFilterIndex = 2;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	// Display the Open dialog box. 

	if (GetOpenFileName(&ofn) == TRUE)
	{
		SetWindowText(GetDlgItem(Dialog, text), ofn.lpstrFile);
		return true;
	}
	else
		MessageBox(Dialog, "No eligió archivo", "Aviso", MB_OK | MB_ICONINFORMATION);
	return false;
}
void addNode(contact aux)
{
	newCont = new contact;
	*newCont = aux;
	if (firstCont == NULL)
	{
		firstCont = newCont;
		lastCont = firstCont;
	}
	else
	{
		lastCont->next = newCont;
		newCont->prev = lastCont;
		lastCont = newCont;
	}
}//Agregar nodo a la lista de contactos
void addNode(date aux)
{
	newDate = new date;
	*newDate = aux;
	if (firstDate == NULL)
	{
		firstDate = newDate;
		lastDate = firstDate;
	}
	else
	{
		lastDate->next = newDate;
		newDate->prev = lastDate;
		lastDate = newDate;
	}
}//Agregar nodo a la lista de citas 
void openFile(HWND ventana, char fileName[], contact *first)
{
	contact aux;
	int size = 0, count = 0;

	fstream reader;
	reader.open(fileName, ios::in | ios::binary);
	if (reader.is_open())
	{
		reader.seekg(0, ios::end);
		size = reader.tellg();
		reader.seekg(0, ios::beg);
		while (count < size)
		{
			reader.read((char*)&aux, sizeof(contact));
			addNode(aux);
			count += sizeof(contact);
		}
		reader.close();
	}
	else
	{
		reader.close();										//Cierra el archivo para poder abrirlo de otra manera
		reader.open(fileName, ios::out | ios::binary | ios::trunc);		//Abre el archivo como salida para poder crear el archivo en caso de que no se pueda abrir
		reader.close();
	}

}//Abrir archivo de contactos
void openFile(HWND ventana, char fileName[], date *first)
{
	date aux;
	int size = 0, count = 0;

	fstream reader;
	reader.open(fileName, ios::in | ios::binary);
	if (reader.is_open())
	{
		reader.seekg(0, ios::end);
		size = reader.tellg();
		reader.seekg(0, ios::beg);
		while (count < size)
		{
			reader.read((char*)&aux, sizeof(date));
			addNode(aux);
			count += sizeof(date);
		}
		reader.close();
	}
	else
	{
		reader.close();													//Cierra el archivo para poder abrirlo de otra manera
		reader.open(fileName, ios::out | ios::binary | ios::trunc);		//Abre el archivo como salida para poder crear el archivo en caso de que no se pueda abrir
		reader.close();
	}

}//Abrir archivo de citas
void openCatsFile(HWND control, char fileName[])
{
	char buffer[20];
	ifstream catsFile;
	catsFile.open(fileName, ios::in | ios::binary);
	if (catsFile.is_open())
	{
		while (!catsFile.eof())
		{
			catsFile >> buffer;
			SendMessage(control, CB_ADDSTRING, 0, (LPARAM)buffer);
		}
		catsFile.close();
	}
}
void saveFile(char fileName[], contact *first)
{
	contact *aux = first;

	ofstream saveConts;
	saveConts.open(fileName, ios::out | ios::binary);
	if (aux != NULL)
	{
		while (aux != NULL)
		{
			saveConts.write((char*)aux, sizeof(contact));
			aux = aux->next;
		}
		saveConts.close();
	}
}
void saveFile(char fileName[], date *first)
{
	date *aux = first;

	ofstream saveConts;
	saveConts.open(fileName, ios::out | ios::binary);
	if (aux != NULL)
	{
		while (aux != NULL)
		{
			saveConts.write((char*)aux, sizeof(date));
			aux = aux->next;
		}
		saveConts.close();
	}
}
void addToCB(HWND control, contact *first)
{
	char fullName[150];

	SendMessage(control, CB_RESETCONTENT, 0, 0);

	contact *aux = first;
	if (aux != NULL)
	{
		while (aux != NULL)
		{
			strcpy(fullName, aux->nombre);
			strcat(fullName, " ");
			strcat(fullName, aux->apellidos);
			SendMessage(control, CB_ADDSTRING, 0, (LPARAM)fullName);
			aux = aux->next;
		}
	}
	else
		SendMessage(control, CB_ADDSTRING, 0, (LPARAM)"No hay contactos");
	
}
void addToLB(HWND control, contact *first)
{
	char fullName[150]{};

	SendMessage(control, LB_RESETCONTENT, 0, 0);

	contact *aux = first;
	if (aux != NULL)
	{
		EnableWindow(control, TRUE);
		while (aux != NULL)
		{
			strcpy(fullName, aux->nombre);
			strcat(fullName, " ");
			strcat(fullName, aux->apellidos);
			SendMessage(control, LB_ADDSTRING, 0, (LPARAM)fullName);
			aux = aux->next;
		}
	}
	else
	{
		SendMessage(control, LB_ADDSTRING, 0, (LPARAM)"No hay contactos");
		EnableWindow(control, FALSE);
	}
}
contact *getSelectedCnt(int cursel)										//Pide como parametro un número entero
{
	contact *auxC = firstCont;											//Crea un puntero auxiliar para recorrer la lista
	int count = 0;														//Declara un contador para comparar con el parametro de la funcion
	while (count != cursel && auxC != NULL)												//El ciclo se va a realizar mientras el contador no sea igual al parametro de la funcion
	{
		auxC = auxC->next;												//Recorre al siguiente elemento de la lista
		count++;														//Aumenta el contador para que se pueda compara correctamente la condicón
	}
		
	return auxC;														//Regresa la dirección de memoria del elemento de la lista
}
void getCntDates(HWND LB_DAYS, HWND LB_TIMES, date *first, char cntPhone[])
{
	bool exist = false;
	char fecha[15]{};
	char tiempo[8]{};
	char dia[3]{}, mes[3]{}, anho[5]{};
	char hora[3]{}, minuto[3]{};
	date *auxiliar = first;

	SendMessage(LB_DAYS, LB_RESETCONTENT, 0, 0);
	SendMessage(LB_TIMES, LB_RESETCONTENT, 0, 0);
	if (first != NULL)
	{
		EnableWindow(LB_DAYS, TRUE);
		EnableWindow(LB_TIMES, TRUE);
		while (auxiliar != NULL)
		{
			if (strcmp(auxiliar->telefono, cntPhone) == 0)
			{
				_itoa(auxiliar->dateAndTime.tm_mday, dia, 10);
				strcpy(fecha, dia);
				strcat(fecha, "/");
				_itoa(auxiliar->dateAndTime.tm_mon, mes, 10);
				strcat(fecha, mes);
				strcat(fecha, "/");
				_itoa(auxiliar->dateAndTime.tm_year, anho, 10);
				strcat(fecha, anho);
				_itoa(auxiliar->dateAndTime.tm_hour, hora, 10);
				strcpy(tiempo, hora);
				strcat(tiempo, ":");
				_itoa(auxiliar->dateAndTime.tm_min, minuto, 10);
				strcat(tiempo, minuto);
				SendMessage(LB_DAYS, LB_ADDSTRING, 0, (LPARAM)fecha);
				SendMessage(LB_TIMES, LB_ADDSTRING, 0, (LPARAM)tiempo);
				exist = true;
			}
			auxiliar = auxiliar->next;
		}
	}
	if (!exist)
	{
		SendMessage(LB_DAYS, LB_ADDSTRING, 0, (LPARAM)"No hay citas");
		SendMessage(LB_TIMES, LB_ADDSTRING, 0, (LPARAM)"No hay citas");
		EnableWindow(LB_DAYS, FALSE);
		EnableWindow(LB_TIMES, FALSE);
	}
}
void deleteContact(contact *node)
{
	if (node->prev == NULL && node->next == NULL)
		lastCont = firstCont = NULL;
	if (node->prev == NULL && node->next != NULL)
	{
		firstCont = node->next;
		firstCont->prev = NULL;
	}
	if (node->prev != NULL && node->next != NULL)
	{
		node->prev->next = node->next;
		node->next->prev = node->prev;
	}

	if (node->next == NULL && node->prev != NULL)
	{
		lastCont = node->prev;
		lastCont->next = NULL;
	}
	delete node;
}
date *getDates(SYSTEMTIME currentTime)
{
	int count = 0;
	date *aux = firstDate;
	if (aux != NULL)
	{
		while (aux != NULL)
		{
			if ((currentTime.wMonth == aux->dateAndTime.tm_mon) && (currentTime.wDay == aux->dateAndTime.tm_mday) && (aux->dateAndTime.tm_year == currentTime.wYear)
				&& (aux->dateAndTime.tm_hour == currentTime.wHour) && (aux->dateAndTime.tm_min == currentTime.wMinute) 
				&& !aux->attended)
				return aux;
			aux = aux->next;
		}
	}
	return 0;
}
void deleteDate(date *node)
{
	if (node->prev == NULL && node->next == NULL)
		lastDate = firstDate = NULL;
	if (node->prev == NULL && node->next != NULL)
	{
		firstDate = node->next;
		firstDate->prev = NULL;
	}
	if (node->prev != NULL && node->next != NULL)
		node->prev->next = node->next;

	if (node->next == NULL && node->prev != NULL)
	{
		lastDate = node->prev;
		lastDate->next = NULL;
	}
	delete node;
}
date *getDateByName(char dateName[])
{
	date *auxR = firstDate;
	if (auxR != NULL)
		while (auxR != NULL)
		{
			if (strcmp(dateName, auxR->dateName) == 0)
				return auxR;
			auxR = auxR->next;
		}
	return 0;
}
void fillWeekCalendar(_SYSTEMTIME selectedDay, HWND hWindow)
{
	date *auxDate = firstDate;
	int firstDay = 0;
	int lastDay = selectedDay.wDay + (6 - selectedDay.wDayOfWeek);
	if (selectedDay.wDay - selectedDay.wDayOfWeek < 1)
		firstDay = 1;
	else
		firstDay = selectedDay.wDay - selectedDay.wDayOfWeek;
	SendMessage(GetDlgItem(hWindow, LB_LUNES), LB_RESETCONTENT, 0, 0);
	SendMessage(GetDlgItem(hWindow, LB_MARTES), LB_RESETCONTENT, 0, 0);
	SendMessage(GetDlgItem(hWindow, LB_MIERCOLES), LB_RESETCONTENT, 0, 0);
	SendMessage(GetDlgItem(hWindow, LB_JUEVES), LB_RESETCONTENT, 0, 0);
	SendMessage(GetDlgItem(hWindow, LB_VIERNES), LB_RESETCONTENT, 0, 0);
	SendMessage(GetDlgItem(hWindow, LB_SABADO), LB_RESETCONTENT, 0, 0);
	if (auxDate != NULL)
	{
		while (auxDate != NULL)
		{
			if ((auxDate->dateAndTime.tm_mday >= firstDay) && (auxDate->dateAndTime.tm_mon == selectedDay.wMonth) && (auxDate->dateAndTime.tm_year == selectedDay.wYear)
			    && (auxDate->dateAndTime.tm_mday <= lastDay))
			{
				if (auxDate->dateAndTime.tm_wday == 0)
					SendMessage(GetDlgItem(hWindow, LB_DOMINGO), LB_ADDSTRING, 0, (LPARAM)auxDate->dateName);
				if (auxDate->dateAndTime.tm_wday == 1)
					SendMessage(GetDlgItem(hWindow, LB_LUNES), LB_ADDSTRING, 0, (LPARAM)auxDate->dateName);
				if (auxDate->dateAndTime.tm_wday == 2)
					SendMessage(GetDlgItem(hWindow, LB_MARTES), LB_ADDSTRING, 0, (LPARAM)auxDate->dateName);
				if (auxDate->dateAndTime.tm_wday == 3)
					SendMessage(GetDlgItem(hWindow, LB_MIERCOLES), LB_ADDSTRING, 0, (LPARAM)auxDate->dateName);
				if (auxDate->dateAndTime.tm_wday == 4)
					SendMessage(GetDlgItem(hWindow, LB_JUEVES), LB_ADDSTRING, 0, (LPARAM)auxDate->dateName);
				if (auxDate->dateAndTime.tm_wday == 5)
					SendMessage(GetDlgItem(hWindow, LB_VIERNES), LB_ADDSTRING, 0, (LPARAM)auxDate->dateName);
				if (auxDate->dateAndTime.tm_wday == 6)
					SendMessage(GetDlgItem(hWindow, LB_SABADO), LB_ADDSTRING, 0, (LPARAM)auxDate->dateName);
			}
			auxDate = auxDate->next;
		}
	}
}
//Main code

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevIns, LPSTR cmdLine, int cmdShow)
{
	hinst = hInst;
	wnHndl = CreateDialog(hInst, MAKEINTRESOURCE(FRM_MAIN), 0, (DLGPROC)mainFRM);
	ShowWindow(wnHndl, cmdShow);


	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while (GetMessage(&msg, 0, 0, 0)) 
	{
		if( mainFRM == 0 || !IsDialogMessage(wnHndl, &msg)) 
		{ 
			if (!IsDialogMessage(wnHndl, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} 
	}
	return (int)msg.wParam;
}
BOOL CALLBACK mainFRM(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int index = 0;

	GetCurrentDirectory(MAX_PATH + 1, contsDirection);
	strcat(contsDirection, "\\contactos.bin");
	GetCurrentDirectory(MAX_PATH + 1, datesDirection);
	strcat(datesDirection, "\\citas.bin");
	GetCurrentDirectory(MAX_PATH + 1, catsDirection);
	strcat(catsDirection, "\\categorias.bin");

	GetCurrentDirectory(MAX_PATH + 1, alarm1Dir);
	strcat(alarm1Dir, "\\cow.mp3");
	GetCurrentDirectory(MAX_PATH + 1, alarm2Dir);
	strcat(alarm2Dir, "\\pop.mp3");
	GetCurrentDirectory(MAX_PATH + 1, alarm3Dir);
	strcat(alarm3Dir, "\\bell.mp3");
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			openFile(hDlg, contsDirection, firstCont);
			openFile(hDlg, datesDirection, firstDate);
			datesTimer = SetTimer(hDlg, TM_DATESTIMER, 10000, (TIMERPROC)NULL);
			return true;
		}
		case WM_TIMER:
		{
			switch (wParam)
			{
				case TM_DATESTIMER:
				{
					GetLocalTime(&currentTime);
					auxDatesTp = getDates(currentTime);
					if (auxDatesTp != NULL && auxDatesTp->attended == false)
					{
						mciSendString("close Alarm", NULL, 0, 0);
						auxDatesTp->attended = true;
						strcpy(dateMessage, "Nombre: ");
						strcat(dateMessage, auxDatesTp->cntName);
						strcat(dateMessage, " ");
						strcat(dateMessage, auxDatesTp->cntAp);
						strcat(dateMessage, "\n");
						strcat(dateMessage, "Relación: ");
						strcat(dateMessage, auxDatesTp->relacion);
						strcat(dateMessage, "\n");
						strcat(dateMessage, "Telefono: ");
						strcat(dateMessage, auxDatesTp->telefono);
						strcat(dateMessage, "\n");
						strcat(dateMessage, "Comentarios: ");
						strcat(dateMessage, auxDatesTp->comment);
						strcat(alarmString, "open \"");
						strcat(alarmString, auxDatesTp->alarm);
						strcat(alarmString, "\" alias Alarm");
						mciSendString(alarmString, NULL, 0, 0);
						mciSendString("play Alarm", NULL, 0, 0);
						int alrmResult = MessageBox(wnHndl, dateMessage, auxDatesTp->dateName, MB_OK | MB_ICONINFORMATION);
						if (alrmResult == IDOK)
							mciSendString("close Alarm", NULL, 0, 0);
					}
				return true;
				}
			 }
			return true;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case BTN_CONTACTOS:
				{
					//Crea el handler para la ventana de AGREGAR CONTACTOS
					hAddCntsFRM = CreateDialog(hinst, MAKEINTRESOURCE(FRM_ADD), wnHndl, (DLGPROC)addCont);
					//Muestra ventana AGREGAR CONTACTOS
					ShowWindow(hAddCntsFRM, SW_SHOW);
					return true;
				}
				case BTN_VERCONTS:
				{
					//Crea handler para la ventana CONTACTOS
					hCntsFRM = CreateDialog(hinst, MAKEINTRESOURCE(FRM_CONTS), 0, (DLGPROC)conts);
					//Abre ventana CONTACTOS
					ShowWindow(hCntsFRM, SW_SHOW);
					return true;
				}
				case BTN_ADDDATES:
				{
					//Crea handler para la ventar AGREGAR CITAS
					hAddDates = CreateDialog(hinst, MAKEINTRESOURCE(FRM_ADDDATE), 0, (DLGPROC)addDate);
					//Abre ventana CITAS 
					ShowWindow(hAddDates, SW_SHOW);
					return true;
				}
				case BTN_CALENDAR:
				{
					//Crea handler para la ventana CALENDARIO SEMANAL
					hCalendar = CreateDialog(hinst, MAKEINTRESOURCE(FRM_CALENDAR), 0, (DLGPROC)calendar);
					//Abre ventana CALENDARIO SEMANAL
					ShowWindow(hCalendar, SW_SHOW);
					return true;
				}
			}
		return true;
		}
		case WM_CLOSE:
		{
			int msbQuit = MessageBox(wnHndl, "Está seguro de que desea salir?", "Aviso", MB_YESNO | MB_ICONEXCLAMATION);
			if (msbQuit == IDYES)
				PostQuitMessage(0);
			return true;
		}
		case WM_DESTROY:
		{
			return true;
		}
	}
	return false;
} /* Ventana principal, terminada*/
LRESULT CALLBACK conts(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int cursel = 0;
	contact **pPointer = &auxCp;
	switch (msg)
	{
		case WM_INITDIALOG:
			addToLB(GetDlgItem(hDlg, LB_CNTSLIST), firstCont);
			openCatsFile(GetDlgItem(hDlg, CMB_REL), catsDirection);
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			   case LB_CNTSLIST:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						//Este pedazo de código limpia todos los comtroles que haya en el formulario
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITNAME), "");
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITAP), "");
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITADRESS), "");
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITMAIL), "");
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITPHONE), "");
						SendMessage(GetDlgItem(hCntsFRM, CMB_REL), CB_SETCURSEL, 0, 0);
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITSONG), "");

						cursel = SendMessage(GetDlgItem(hDlg, LB_CNTSLIST), LB_GETCURSEL, NULL, NULL);  //Esta parte recoge el indice seleccionado en el listbox 

						auxCp = getSelectedCnt(cursel);													//Le pasa como parametro el indice a la funcion "getSelectedCnt() y la direccion que devuele lo pasa a un puntero"
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITNAME), auxCp->nombre);				//Empieza a setear el testo de los edit boxes y demás controles
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITAP), auxCp->apellidos);
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITADRESS), auxCp->direccion);
						if (auxCp->genero)
							SendMessage(GetDlgItem(hCntsFRM, RB_MALE), BM_SETCHECK, BST_CHECKED, 0);
						else
							SendMessage(GetDlgItem(hCntsFRM, RB_FEMALE), BM_SETCHECK, BST_CHECKED, 0);
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITMAIL), auxCp->correo);
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITPHONE), auxCp->telefono);
						SendMessage(GetDlgItem(hCntsFRM, CMB_REL), CB_SETCURSEL, 0, auxCp->relacion);

						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITSONG), auxCp->cancion);
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITPHOTO), auxCp->foto1);
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITPHOTO2), auxCp->foto2);

						editPic1 = (HBITMAP)LoadImage(NULL, auxCp->foto1, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
						SendMessage(GetDlgItem(hCntsFRM, PB_USERPHOTOSEDIT), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)editPic1);
						GetWindowText(GetDlgItem(hCntsFRM, TXT_EDITSONG), auxCp->cancion, sizeof(auxCp->cancion));
						mciSendString("close Song", 0, 0, 0);
						strcpy(bufferString, "open \"");
						strcat(bufferString, auxCp->cancion);
						strcat(bufferString, "\" alias Song");
						mciSendString(bufferString, 0, 0, 0);
						getCntDates(GetDlgItem(hCntsFRM, LB_DATES), GetDlgItem(hCntsFRM, LB_TIME), firstDate, auxCp->telefono);
					}
				break;
			   case CB_EDIT:
					   if (SendDlgItemMessage(hCntsFRM, CB_EDIT, BM_GETCHECK, 0, 0) == BST_CHECKED && auxCp != NULL)
					   {
						   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITNAME), TRUE);
						   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITAP), TRUE);
						   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITADRESS), TRUE);
						   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITMAIL), TRUE);
						   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITPHONE), TRUE);
						   EnableWindow(GetDlgItem(hCntsFRM, BTN_EDITSONG), TRUE);
						   EnableWindow(GetDlgItem(hCntsFRM, CMB_REL), TRUE);
						   EnableWindow(GetDlgItem(hCntsFRM, BTN_SAVE), TRUE);
						   EnableWindow(GetDlgItem(hCntsFRM, BTN_DELETE), TRUE);
						   EnableWindow(GetDlgItem(hCntsFRM, BTN_EDITPHOTO), TRUE);
						   EnableWindow(GetDlgItem(hCntsFRM, BTN_PREVE), TRUE);
						   EnableWindow(GetDlgItem(hCntsFRM, BTN_NEXTE), TRUE);
						   EnableWindow(GetDlgItem(hCntsFRM, BTN_EDITPHOTO), TRUE);
						   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITNAME), FALSE);
						   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITAP), FALSE);
						   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITADRESS), FALSE);
						   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITMAIL), FALSE);
						   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITPHONE), FALSE);
					   }
					   else
					   {
						   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITNAME), FALSE);
						   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITAP), FALSE);
						   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITADRESS), FALSE);
						   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITMAIL), FALSE);
						   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITPHONE), FALSE);
						   EnableWindow(GetDlgItem(hCntsFRM, BTN_EDITSONG), FALSE);
						   EnableWindow(GetDlgItem(hCntsFRM, CMB_REL), FALSE);
						   EnableWindow(GetDlgItem(hCntsFRM, BTN_SAVE), FALSE);
						   EnableWindow(GetDlgItem(hCntsFRM, BTN_DELETE), FALSE);
						   EnableWindow(GetDlgItem(hCntsFRM, BTN_EDITPHOTO), FALSE);
						   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITNAME), TRUE);
						   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITAP), TRUE);
						   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITADRESS), TRUE);
						   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITMAIL), TRUE);
						   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITPHONE), TRUE);
					   }
				 break;
			   case BTN_SAVE:
				   if (auxCp != NULL)
				   {
					   GetWindowText(GetDlgItem(hCntsFRM, TXT_EDITNAME), auxCp->nombre, sizeof(auxCp->nombre));
					   GetWindowText(GetDlgItem(hCntsFRM, TXT_EDITAP), auxCp->apellidos, sizeof(auxCp->apellidos));
					   GetWindowText(GetDlgItem(hCntsFRM, TXT_EDITADRESS), auxCp->direccion, sizeof(auxCp->direccion));
					   GetWindowText(GetDlgItem(hCntsFRM, TXT_EDITMAIL), auxCp->correo, sizeof(auxCp->correo));
					   GetWindowText(GetDlgItem(hCntsFRM, TXT_EDITPHONE), auxCp->telefono, sizeof(auxCp->telefono));
					   GetWindowText(GetDlgItem(hCntsFRM, TXT_EDITPHOTO), auxCp->foto1, sizeof(auxCp->foto1));
					   GetWindowText(GetDlgItem(hCntsFRM, TXT_EDITPHOTO2), auxCp->foto2, sizeof(auxCp->foto2));
					   auxCp->relacion = SendMessage(GetDlgItem(hCntsFRM, CMB_REL), CB_GETCURSEL, 0, 0);
					   GetWindowText(GetDlgItem(hCntsFRM, TXT_EDITSONG), auxCp->cancion, sizeof(auxCp->cancion));
					   mciSendString("close Song", 0, 0, 0);
					   saveFile(contsDirection, firstCont);
					   addToLB(GetDlgItem(hCntsFRM, LB_CNTSLIST), firstCont);
				   }
				   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITNAME), FALSE);
				   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITAP), FALSE);
				   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITADRESS), FALSE);
				   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITMAIL), FALSE);
				   EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITPHONE), FALSE);
				   EnableWindow(GetDlgItem(hCntsFRM, BTN_EDITSONG), FALSE);
				   EnableWindow(GetDlgItem(hCntsFRM, CMB_REL), FALSE);
				   EnableWindow(GetDlgItem(hCntsFRM, BTN_SAVE), FALSE);
				   EnableWindow(GetDlgItem(hCntsFRM, BTN_EDITPHOTO), FALSE);
				   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITNAME), TRUE);
				   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITAP), TRUE);
				   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITADRESS), TRUE);
				   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITMAIL), TRUE);
				   Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITPHONE), TRUE);
				   CheckDlgButton(hCntsFRM, CB_EDIT, BST_UNCHECKED);
			    break;
			    case BTN_DELETE:
					if (auxCp != NULL)
					{
						mciSendString("close Song", 0, 0, 0);
						deleteContact(auxCp);
						saveFile(contsDirection, firstCont);
						addToLB(GetDlgItem(hDlg, LB_CNTSLIST), firstCont);

						EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITNAME), FALSE);
						EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITAP), FALSE);
						EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITADRESS), FALSE);
						EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITMAIL), FALSE);
						EnableWindow(GetDlgItem(hCntsFRM, TXT_EDITPHONE), FALSE);
						EnableWindow(GetDlgItem(hCntsFRM, BTN_EDITSONG), FALSE);
						EnableWindow(GetDlgItem(hCntsFRM, CMB_REL), FALSE);
						EnableWindow(GetDlgItem(hCntsFRM, BTN_SAVE), FALSE);
						Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITNAME), TRUE);
						Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITAP), TRUE);
						Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITADRESS), TRUE);
						Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITMAIL), TRUE);
						Edit_SetReadOnly(GetDlgItem(hCntsFRM, TXT_EDITPHONE), TRUE);
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITNAME), "");
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITAP), "");
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITPHONE), "");
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITMAIL), "");
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITADRESS), "");
						SendMessage(GetDlgItem(hCntsFRM, CB_EDIT), BM_SETCHECK, BST_UNCHECKED, 0);
						SendMessage(GetDlgItem(hCntsFRM, CMB_REL), CB_SETCURSEL, 0, 0);
						SetWindowText(GetDlgItem(hCntsFRM, TXT_EDITSONG), "");
					}
				break;
				case BTN_EDITPHOTO:
					if (pIndex == 0)
						SelectFile(hCntsFRM, TXT_EDITPHOTO, 1);
					else
						SelectFile(hCntsFRM, TXT_EDITPHOTO2, 1);
				break;
				case BTN_PREVE:
					if (pIndex == 1)
						pIndex = 0;
					editPic1 = (HBITMAP)LoadImage(NULL, auxCp->foto1, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
					SendMessage(GetDlgItem(hCntsFRM, PB_USERPHOTOSEDIT), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)editPic1);
				break;
				case BTN_NEXTE:
					if (pIndex == 0)
						pIndex = 1;
					editPic2 = (HBITMAP)LoadImage(NULL, auxCp->foto2, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
					SendMessage(GetDlgItem(hCntsFRM, PB_USERPHOTOSEDIT), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)editPic2);
				break;
				case BTN_EDITSONG:
					mciSendString("close Song", 0, 0, 0);
					if (SelectFile(hCntsFRM, TXT_EDITSONG, 2))
					{
						GetWindowText(GetDlgItem(hCntsFRM, TXT_EDITSONG), auxCp->cancion, sizeof(auxCp->cancion));
						strcpy(bufferString, "open \"");
						strcat(bufferString, auxCp->cancion);
						strcat(bufferString, "\" alias Song");
						mciSendString(bufferString, 0, 0, 0);
					}
				break;
				case BTN_PLAYE:
					mciSendString("play Song", 0, 0, 0);
				break;
				case BTN_PAUSEE:
					mciSendString("pause Song", 0, 0, 0);
				break;
			}
		break;
		case WM_CLOSE:
			mciSendString("close Song", 0, 0, 0);
			*pPointer = NULL;
			DestroyWindow(hCntsFRM);
		break;
		case WM_DESTROY:
		break;
	}
	return false;
}
LRESULT CALLBACK addCont(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		pIndex = 0;
		openCatsFile(GetDlgItem(hDlg, CB_REL), "categorias.bin");
		addToLB(GetDlgItem(hDlg, LB_CNTS), firstCont);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BTN_ADDSONG:
				mciSendString("close song", 0, 0, 0);
				SelectFile(hAddCntsFRM, TXT_SONG, 2);
				GetWindowText(GetDlgItem(hAddCntsFRM, TXT_SONG), auxCont.cancion, MAX_PATH);
				strcpy(bufferString, "open \"");
				strcat(bufferString, auxCont.cancion);
				strcat(bufferString, "\" alias Song");
				mciSendString(bufferString, NULL, 0, 0);
			break;
		case BTN_PLAY:
				mciSendString("play Song", 0, 0, 0);
			break;
		case BTN_PAUSE:
				mciSendString("pause Song", 0, 0, 0);
			break;
		case BTN_PREV:
			pIndex = 0;
			GetWindowText(GetDlgItem(hAddCntsFRM, TXT_PHOTO), auxCont.foto1, MAX_PATH);
			pic1 = (HBITMAP)LoadImage(NULL, auxCont.foto1, IMAGE_BITMAP, 200, 200, LR_LOADFROMFILE);
			SendMessage(GetDlgItem(hAddCntsFRM, PB_USERPHOTOS), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)pic1);
			break;
		case BTN_NEXT:
				pIndex = 1;
				GetWindowText(GetDlgItem(hAddCntsFRM, TXT_PHOTO2), auxCont.foto2, MAX_PATH);
				pic2 = (HBITMAP)LoadImage(NULL, auxCont.foto2, IMAGE_BITMAP, 200, 200, LR_LOADFROMFILE);
				SendMessage(GetDlgItem(hAddCntsFRM, PB_USERPHOTOS), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)pic2);
			break;
		case BTN_ADDPHOTO:
			if (pIndex == 0)
			{
				SelectFile(hAddCntsFRM, TXT_PHOTO, 1);
				GetWindowText(GetDlgItem(hAddCntsFRM, TXT_PHOTO), auxCont.foto1, MAX_PATH);
				pic1 = (HBITMAP)LoadImage(NULL, auxCont.foto1, IMAGE_BITMAP, 200, 200, LR_LOADFROMFILE);
				SendMessage(GetDlgItem(hAddCntsFRM, PB_USERPHOTOS), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)pic1);
			}
			if (pIndex == 1)
			{
				SelectFile(hAddCntsFRM, TXT_PHOTO2, 1);
				GetWindowText(GetDlgItem(hAddCntsFRM, TXT_PHOTO2), auxCont.foto2, MAX_PATH);
				pic2 = (HBITMAP)LoadImage(NULL, auxCont.foto2, IMAGE_BITMAP, 200, 200, LR_LOADFROMFILE);
				SendMessage(GetDlgItem(hAddCntsFRM, PB_USERPHOTOS), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)pic2);
			}
			break;
		case BTN_ADD:
			GetWindowText(GetDlgItem(hAddCntsFRM, TXT_NAME), auxCont.nombre, 50);
			//Detecta si el primer carácter del arreglo de nombre es vacío
			if (auxCont.nombre[0] == '\0' || auxCont.nombre[0] == ' ') emptyField = true; else emptyField = false;
				GetWindowText(GetDlgItem(hAddCntsFRM, TXT_AP), auxCont.apellidos, 50);
				if (auxCont.apellidos[0] == '\0' || auxCont.apellidos[0] == ' ') emptyField = true;
				GetWindowText(GetDlgItem(hAddCntsFRM, TXT_ADRESS), auxCont.direccion, 75);
				if (auxCont.direccion[0] == '\0' || auxCont.direccion[0] == ' ') emptyField = true; else emptyField = false;
				GetWindowText(GetDlgItem(hAddCntsFRM, TXT_PHONE), auxCont.telefono, 10);
				if (auxCont.telefono[0] == '\0' || auxCont.telefono[0] == ' ') emptyField = true; else emptyField = false;
				GetWindowText(GetDlgItem(hAddCntsFRM, TXT_MAIL), auxCont.correo, 50);
				if (SendMessage(GetDlgItem(hAddCntsFRM, RB_MALE), BM_GETCHECK, 0, 0) == BST_CHECKED)
					auxCont.genero = true;
				else
					auxCont.genero = false;
				auxCont.relacion = SendMessage(GetDlgItem(hAddCntsFRM, CB_REL), CB_GETCURSEL, 0, 0);
				if (GetWindowText(GetDlgItem(hAddCntsFRM, TXT_SONG), auxCont.cancion, sizeof(auxCont.cancion)) == 0) emptyField = true; else emptyField = false;
				if (!emptyField)
				{
					mciSendString("close Song", 0, 0, 0);
					addNode(auxCont);
					addToLB(GetDlgItem(hAddCntsFRM, LB_CNTS), firstCont);
					saveFile(contsDirection, firstCont);
				}
				else
					MessageBox(hAddCntsFRM, "Por favor llene todos los campos", "Aviso", MB_OK | MB_ICONASTERISK);
				SetWindowText(GetDlgItem(hAddCntsFRM, TXT_NAME), "");
				SetWindowText(GetDlgItem(hAddCntsFRM, TXT_AP), "");
				SetWindowText(GetDlgItem(hAddCntsFRM, TXT_ADRESS), "");
				SetWindowText(GetDlgItem(hAddCntsFRM, TXT_PHONE), "");
				SetWindowText(GetDlgItem(hAddCntsFRM, TXT_MAIL), "");
				SendMessage(GetDlgItem(hAddCntsFRM, CB_REL), CB_SETCURSEL, 0, 0);
				SetWindowText(GetDlgItem(hAddCntsFRM, TXT_SONG), "");
			}
			break;
		case WM_CLOSE:
			mciSendString("close Song", 0, 0, 0);
			strcpy(bufferString, "");
			auxCont = {};
			DestroyWindow(hAddCntsFRM);
			break;
		case WM_DESTROY:
			break;
		}
	return false;
}
LRESULT CALLBACK addDate(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int cursel = 0, alarmCursel = 0;
	switch (msg)
	{
		case WM_INITDIALOG:
			addToLB(GetDlgItem(hDlg, LB_DATECNTS), firstCont);
			openCatsFile(GetDlgItem(hDlg, CB_DATEREL), catsDirection);
			SendMessage(GetDlgItem(hDlg, CB_ALARM), CB_ADDSTRING, 0, (LPARAM)"Canción favorita");
			SendMessage(GetDlgItem(hDlg, CB_ALARM), CB_ADDSTRING, 0, (LPARAM)"Vaquita");
			SendMessage(GetDlgItem(hDlg, CB_ALARM), CB_ADDSTRING, 0, (LPARAM)"Pop-up");
			SendMessage(GetDlgItem(hDlg, CB_ALARM), CB_ADDSTRING, 0, (LPARAM)"Campanas");
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case LB_DATECNTS:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						EnableWindow(GetDlgItem(hAddDates, BTN_SAVEDATE), TRUE);
						EnableWindow(GetDlgItem(hAddDates, CB_ALARM), TRUE);
						SendMessage(GetDlgItem(hAddDates, CB_ALARM), CB_SETCURSEL, 0, 0);
						SetWindowText(GetDlgItem(hAddDates, TXT_DATENAME), "");
						SetWindowText(GetDlgItem(hAddDates, TXT_DATEAP), "");
						SendMessage(GetDlgItem(hAddDates, CB_DATEREL), CB_SETCURSEL, 0, 0);
						SetWindowText(GetDlgItem(hAddDates, TXT_DATETEL), "");
						SendMessage(GetDlgItem(hAddDates, CB_ALARM), CB_SETCURSEL, 0, 0);

						cursel = SendMessage(GetDlgItem(hDlg, LB_DATECNTS), LB_GETCURSEL, NULL, NULL); 

						auxCp = getSelectedCnt(cursel);
						SetWindowText(GetDlgItem(hAddDates, TXT_DATENAME), auxCp->nombre);
						SetWindowText(GetDlgItem(hAddDates, TXT_DATEAP), auxCp->apellidos);
						SendMessage(GetDlgItem(hAddDates, CB_DATEREL), CB_SETCURSEL, auxCp->relacion, 0);
						SetWindowText(GetDlgItem(hAddDates, TXT_DATETEL), auxCp->telefono);
						SendMessage(GetDlgItem(hAddDates, CB_ALARM), CB_SETCURSEL, 0, 0);
					}
				break;
				case BTN_SAVEDATE:
					GetLocalTime(&currentTime);
					if (GetWindowText(GetDlgItem(hAddDates, TXT_NAMEDATE), auxDate.dateName, sizeof(auxDate.dateName)) > 0)
					{
						SendMessage(GetDlgItem(hAddDates, MC_DATE), MCM_GETCURSEL, 0, (LPARAM)&dateDay);
						SendMessage(GetDlgItem(hAddDates, DTP_TIME), DTM_GETSYSTEMTIME, 0, (LPARAM)&dateTime);
						if ((dateDay.wDay >= currentTime.wDay) && (dateDay.wMonth >= currentTime.wMonth) && (dateDay.wYear >= currentTime.wYear)
							&& (dateTime.wHour >= currentTime.wHour && dateTime.wMinute >= currentTime.wMinute))
						{
							auxDate.dateAndTime.tm_mday = dateDay.wDay;
							auxDate.dateAndTime.tm_wday = dateDay.wDayOfWeek;
							auxDate.dateAndTime.tm_mon = dateDay.wMonth;
							auxDate.dateAndTime.tm_year = dateDay.wYear;

							auxDate.dateAndTime.tm_hour = dateTime.wHour;
							auxDate.dateAndTime.tm_min = dateTime.wMinute;
							GetWindowText(GetDlgItem(hAddDates, TXT_DATENAME), auxDate.cntName, sizeof(auxDate.cntName));
							GetWindowText(GetDlgItem(hAddDates, TXT_DATEAP), auxDate.cntAp, sizeof(auxDate.cntAp));
							GetWindowText(GetDlgItem(hAddDates, TXT_DATETEL), auxDate.telefono, sizeof(auxDate.telefono));
							GetWindowText(GetDlgItem(hAddDates, TXT_DATEDETAILS), auxDate.comment, sizeof(auxDate.comment));
							SendMessage(GetDlgItem(hAddDates, CB_DATEREL), CB_GETLBTEXT, SendMessage(GetDlgItem(hAddDates, CB_DATEREL), CB_GETCURSEL, 0, 0), (LPARAM)auxDate.relacion);
							alarmCursel = SendMessage(GetDlgItem(hAddDates, CB_ALARM), CB_GETCURSEL, 0, 0);
							if (alarmCursel == 0)
								strcpy(auxDate.alarm, auxCp->cancion);
							if (alarmCursel == 1)
								strcpy(auxDate.alarm, alarm1Dir);
							if (alarmCursel == 2)
								strcpy(auxDate.alarm, alarm2Dir);
							if (alarmCursel == 3)
								strcpy(auxDate.alarm, alarm3Dir);

							addNode(auxDate);
							saveFile(datesDirection, firstDate);
							ZeroMemory(&auxDate, sizeof(auxDate));
							SetWindowText(GetDlgItem(hAddDates, TXT_NAMEDATE), "");
							SetWindowText(GetDlgItem(hAddDates, TXT_DATENAME), "");
							SetWindowText(GetDlgItem(hAddDates, TXT_DATEAP), "");
							SetWindowText(GetDlgItem(hAddDates, TXT_DATETEL), "");
							SetWindowText(GetDlgItem(hAddDates, TXT_DATEDETAILS), "");
						}
						else
							MessageBox(hAddDates, "Capture una fecha y hora valida, posterior al día de hoy y a la hora actual", "Aviso", MB_OK | MB_ICONASTERISK);
					}
					else
						MessageBox(hAddDates, "El Nombre de la cita es obligatorio", "Aviso", MB_OK | MB_ICONSTOP);
				break;
			}
		break;
		case WM_CLOSE:
			DestroyWindow(hAddDates);
		break;
		case WM_DESTROY:
		break;
	}
	return false;
}
LRESULT CALLBACK editDate(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	_SYSTEMTIME newDate;
	_SYSTEMTIME newTime;
	switch (msg)
	{
		case WM_INITDIALOG:
			openCatsFile(GetDlgItem(hDlg, CB_EDATEREL), catsDirection);
			SendMessage(GetDlgItem(hDlg, CB_EALARM), CB_ADDSTRING, 0, (LPARAM)"Canción favorita");
			SendMessage(GetDlgItem(hDlg, CB_EALARM), CB_ADDSTRING, 0, (LPARAM)"Vaquita");
			SendMessage(GetDlgItem(hDlg, CB_EALARM), CB_ADDSTRING, 0, (LPARAM)"Pop-up");
			SendMessage(GetDlgItem(hDlg, CB_EALARM), CB_ADDSTRING, 0, (LPARAM)"Campanas");
			SetWindowText(GetDlgItem(hDlg, TXT_ENAMEDATE), auxCal->dateName);
			SetWindowText(GetDlgItem(hDlg, TXT_EDATENAME), auxCal->cntName);
			SetWindowText(GetDlgItem(hDlg, TXT_EDATEAP), auxCal->cntAp);
			//SendMessage(GetDlgItem(hDlg, CB_EDATEREL), CB_ADDSTRING, 0, (LPARAM)auxCal->relacion);
			SetWindowText(GetDlgItem(hDlg, TXT_EDATETEL), auxCal->telefono);
			SetWindowText(GetDlgItem(hDlg, TXT_EDATEDETAILS), auxCal->comment);
			SendMessage(GetDlgItem(hDlg, CB_EALARM), CB_SETCURSEL, auxCal->alarmCursel, 0);
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case BTN_SAVEEDITDATE:
					SendMessage(GetDlgItem(hEditDate, MC_EDIT), MCM_GETCURSEL, 0, (LPARAM)&newDate);
					SendMessage(GetDlgItem(hEditDate, DTP_EDIT), DTM_GETSYSTEMTIME, 0, (LPARAM)&newTime);
					if ((newDate.wDay >= auxCal->dateAndTime.tm_mday) && (newDate.wMonth >= auxCal->dateAndTime.tm_mon) && (newDate.wYear >= auxCal->dateAndTime.tm_year)
						&& ((newTime.wHour >= auxCal->dateAndTime.tm_hour) && (newTime.wMinute > auxCal->dateAndTime.tm_min)))
					{
						if (GetWindowText(GetDlgItem(hEditDate, TXT_ENAMEDATE), auxCal->dateName, sizeof(auxCal->dateName)) == 0) emptyField = true; else emptyField = false;
						if (GetWindowText(GetDlgItem(hEditDate, TXT_EDATETEL), auxCal->telefono, sizeof(auxCal->telefono)) == 0) emptyField = true; else emptyField = false;
						if (GetWindowText(GetDlgItem(hEditDate, TXT_EDATEDETAILS), auxCal->comment, sizeof(auxCal->comment)) == 0) emptyField = true; else emptyField = false;
						auxCal->alarmCursel = SendMessage(GetDlgItem(hDlg, CB_EALARM), CB_GETCURSEL, 0, 0);
						if (auxCal->alarmCursel == 0)
							strcpy(auxCal->alarm, auxCal->alarm);
						if (auxCal->alarmCursel == 1)
							strcpy(auxCal->alarm, alarm1Dir);
						if (auxCal->alarmCursel == 2)
							strcpy(auxCal->alarm, alarm2Dir);
						if (auxCal->alarmCursel == 3)
							strcpy(auxCal->alarm, alarm3Dir);
						if (!emptyField)
						{
							saveFile(datesDirection, firstDate);
							DestroyWindow(hEditDate);
						}
						else
							MessageBox(hEditDate, "Por favor complete todos los campos", "Aviso", MB_OK | MB_ICONINFORMATION);
					}
					else
						MessageBox(hEditDate, "Ingrese una fecha y hora posterior al día y la hora actual", "Aviso", MB_OK | MB_ICONINFORMATION);
				break;
			}
		break;
		case WM_CLOSE:
			GetLocalTime(&currentTime);
			fillWeekCalendar(currentTime, hCalendar);
			DestroyWindow(hEditDate);
		break;
	}
	return false;
}
LRESULT CALLBACK calendar(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	_SYSTEMTIME current;
	switch (msg)
	{
		case WM_INITDIALOG:
			GetLocalTime(&current);
			fillWeekCalendar(current, hDlg);
		break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case BTN_SELDAY:
					SendMessage(GetDlgItem(hCalendar, MC_PICKDDATE), MCM_GETCURSEL, 0, (LPARAM)&current);
					fillWeekCalendar(current, hCalendar);
				break;
				case LB_LUNES:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						SendMessage(GetDlgItem(hCalendar, LB_LUNES), LB_GETTEXT, SendMessage(GetDlgItem(hCalendar, LB_LUNES), LB_GETCURSEL, 0, 0), (LPARAM)dateName);
						EnableWindow(GetDlgItem(hCalendar, BTN_DELDATE), TRUE);
						EnableWindow(GetDlgItem(hCalendar, BTN_EDITDATE), TRUE);
					}
				break;
				case LB_MARTES:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						SendMessage(GetDlgItem(hCalendar, LB_MARTES), LB_GETTEXT, SendMessage(GetDlgItem(hCalendar, LB_MARTES), LB_GETCURSEL, 0, 0), (LPARAM)dateName);
						EnableWindow(GetDlgItem(hCalendar, BTN_DELDATE), TRUE);
						EnableWindow(GetDlgItem(hCalendar, BTN_EDITDATE), TRUE);
					}
				break;
				case LB_MIERCOLES:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						SendMessage(GetDlgItem(hCalendar, LB_MIERCOLES), LB_GETTEXT, SendMessage(GetDlgItem(hCalendar, LB_MIERCOLES), LB_GETCURSEL, 0, 0), (LPARAM)dateName);
						EnableWindow(GetDlgItem(hCalendar, BTN_DELDATE), TRUE);
						EnableWindow(GetDlgItem(hCalendar, BTN_EDITDATE), TRUE);
					}
				break;
				case LB_JUEVES:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						SendMessage(GetDlgItem(hCalendar, LB_JUEVES), LB_GETTEXT, SendMessage(GetDlgItem(hCalendar, LB_JUEVES), LB_GETCURSEL, 0, 0), (LPARAM)dateName);
						EnableWindow(GetDlgItem(hCalendar, BTN_DELDATE), TRUE);
						EnableWindow(GetDlgItem(hCalendar, BTN_EDITDATE), TRUE);
					}
					break;
				case LB_VIERNES:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						SendMessage(GetDlgItem(hCalendar, LB_VIERNES), LB_GETTEXT, SendMessage(GetDlgItem(hCalendar, LB_VIERNES), LB_GETCURSEL, 0, 0), (LPARAM)dateName);
						EnableWindow(GetDlgItem(hCalendar, BTN_DELDATE), TRUE);
						EnableWindow(GetDlgItem(hCalendar, BTN_EDITDATE), TRUE);
					}
					break;
				case LB_SABADO:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						SendMessage(GetDlgItem(hCalendar, LB_SABADO), LB_GETTEXT, SendMessage(GetDlgItem(hCalendar, LB_SABADO), LB_GETCURSEL, 0, 0), (LPARAM)dateName);
						EnableWindow(GetDlgItem(hCalendar, BTN_DELDATE), TRUE);
						EnableWindow(GetDlgItem(hCalendar, BTN_EDITDATE), TRUE);
					}
					break;
				case LB_DOMINGO:
					if (HIWORD(wParam) == LBN_DBLCLK)
					{
						SendMessage(GetDlgItem(hCalendar, LB_DOMINGO), LB_GETTEXT, SendMessage(GetDlgItem(hCalendar, LB_DOMINGO), LB_GETCURSEL, 0, 0), (LPARAM)dateName);
						EnableWindow(GetDlgItem(hCalendar, BTN_DELDATE), TRUE);
						EnableWindow(GetDlgItem(hCalendar, BTN_EDITDATE), TRUE);
					}
					break;
				case BTN_DELDATE:
					auxCal = getDateByName(dateName);
					if (auxCal != NULL)
					{
						deleteDate(auxCal);
						EnableWindow(GetDlgItem(hCalendar, BTN_DELDATE), FALSE);
						EnableWindow(GetDlgItem(hCalendar, BTN_EDITDATE), FALSE);
						saveFile(datesDirection, firstDate);
						GetLocalTime(&current);
						fillWeekCalendar(current, hCalendar);
					}
				break;
				case BTN_EDITDATE:
					auxCal = getDateByName(dateName);
					if (auxCal != NULL)
					{
						hEditDate = CreateDialog(hinst, MAKEINTRESOURCE(FRM_EDITDATE), NULL, (DLGPROC)editDate);
						ShowWindow(hEditDate, 1);
					}
				break;
			}
		break;
		
		case WM_CLOSE:
			DestroyWindow(hCalendar);
		break;
		case WM_DESTROY:
		break;
	}
	return false;
}