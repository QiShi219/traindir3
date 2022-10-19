/*	espanol.cpp - Created by Giampiero Caprino

This file is part of Train Director 3

Train Director is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; using exclusively version 2.
It is expressly forbidden the use of higher versions of the GNU
General Public License.

Train Director is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Train Director; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "Traindir3.h"
#include "localize.h"

LocalizeInfo espanol[] = {

	{ wxT("Arrival"), wxT("Llegada") },
	{ wxT("From"), wxT("De") },
	{ wxT("To"), wxT("A") },
	{ wxT("Train"), wxT("Tren") },
	{ wxT("Speed"), wxT("Velocidad") },
	{ wxT("Min.Del."), wxT("Ret.Ingr.") },
	{ wxT("Min.Late"), wxT("Retraso") },
	{ wxT("Status"), wxT("Estado") },

	{ wxT("Entry"), wxT("Entrada") },
	{ wxT("Exit"), wxT("Salida") },
	{ wxT("Entry/exit"), wxT("Entrada/Salida") },
	{ wxT("Times"), wxT("Hora") },

	{ wxT("Performance Statistics"), wxT("Estadistica") },
	{ wxT("Save"), wxT("Guardar") },

	{ wxT("Departing trains"), wxT("Trenes saliendo") },
	{ wxT("Train Schedule"), wxT("Horario del tren") },

	{ wxT("Close"), wxT("Cerrar") },
	{ wxT("Stats"), wxT("Estad韘t.") },
	{ wxT("Print..."), wxT("Imprimir...") },
	{ wxT("Assign"), wxT("Asignar") },
	{ wxT("Shunt"), wxT("Maniobra") },
	{ wxT("Cancel"), wxT("Cancelar") },
	{ wxT("Properties"), wxT("Propiedades") },
	{ wxT("Hide/Show titles"), wxT("Esconder/Mostrar t韙ulos") },

	{ wxT("Name"), wxT("Nombre") },
	{ wxT("Sections"), wxT("Secci髇") },

	{ wxT("Track properties"), wxT("Propriedad del tramo") },
	{ wxT("Track %d,%d"), wxT("Tramo %d,%d") },
	{ wxT("Length :"), wxT("Longitud ") },
	{ wxT("Speed :"), wxT("Velocidad") },
	{ wxT("Station :"), wxT("Estaci髇") },
	{ wxT("Linked to west :"), wxT("Asociar al oeste :") },
	{ wxT("Controls :"), wxT("Controles") },
	{ wxT("Linked to east :"), wxT("Asociar al este :") },

	{ wxT("Trigger"), wxT("Pedal") },
	{ wxT("Trigger properties"), wxT("Propiedades pedal") },
	{ wxT("Trigger at  %d,%d"), wxT("Pedal a  %d,%d") },
	{ wxT("Action :"), wxT("Acci髇 :") },
	{ wxT("'@' in action = name of triggering train"), wxT("'@' en acci髇 = nombre del tren activante") },
	{ wxT("Linked to track at coord:"), wxT("Asociar al tramo :") },
	{ wxT("Probabilities for action :"), wxT("Probabilidades de la acci髇 :") },

	{ wxT("Signal properties"), wxT("Propiedades de la se馻l") },
	{ wxT("Signal at %d,%d"), wxT("Se馻l en %d,%d") },
	{ wxT("Signal Name :"), wxT("Nombre de la se馻l") },
	{ wxT("Signal is always red"), wxT("Se馻l siempre roja") },
	{ wxT("Signal has square frame"), wxT("Se馻l de forma cuadrada") },
	{ wxT("No penalty for train stopping at this signal"), wxT("No penalizar trenes parados en la se馻l") },

	{ wxT("Territory Information"), wxT("Informaci髇 del territorio") },
	{ wxT("Update"), wxT("Actualizar") },
	{ wxT("Print"), wxT("Imprimir") },

	{ wxT("Schedule"), wxT("Horario") },
	{ wxT("Alerts"), wxT("Alertas") },
	{ wxT("Train Info"), wxT("Informaci髇 del tren") },

	{ wxT("Day Selection"), wxT("Elija d") },
	{ wxT("Not all trains run every day of the week.\nWhich day do you want to simulate?"), wxT("No todos los trenes circulan todos los  quiere simular?") },
	{ wxT("Monday"), wxT("Lunes") },
	{ wxT("Tuesday"), wxT("Martes") },
	{ wxT("Wednesday"), wxT("Mi閞coles") },
	{ wxT("Thursday"), wxT("Jueves") },
	{ wxT("Friday"), wxT("Viernes") },
	{ wxT("Saturday"), wxT("S醔ado") },
	{ wxT("Sunday"), wxT("Domingo") },
	{ wxT("Mon"), wxT("Lun") },
	{ wxT("Tue"), wxT("Mar") },
	{ wxT("Wed"), wxT("Mi") },
	{ wxT("Thu"), wxT("Jue") },
	{ wxT("Fri"), wxT("Vie") },
	{ wxT("Sat"), wxT("S醔") },
	{ wxT("Sun"), wxT("Dom") },

	{ wxT("Write default preferences to ini file?"), wxT("縀scribir las preferencias en el archivo ini?") },
	{ wxT("Preferences"), wxT("Preferencias") },
	{ wxT("Full status info"), wxT("Informaci髇 del estado completo") },
	{ wxT("Status line at top of window"), wxT("L韓ea de estado arriba") },
	{ wxT("Alert sound on"), wxT("Sonido de aviso activado") },
	{ wxT("Beep on train entering scenario"), wxT("Soner韆 al entrar un tren") },
	{ wxT("View speed limits"), wxT("Mostrar l韒ite de velocidad") },
	{ wxT("Automatically link signals"), wxT("Asociar se馻les autom醫icamente") },
	{ wxT("Show grid"), wxT("Mostrar cuadr韈ula") },
	{ wxT("View long blocks"), wxT("Mostrar tramos largos") },
	{ wxT("Show seconds on clock"), wxT("Mostrar los segundos en el reloj") },
	{ wxT("Italian signals"), wxT("Se馻les italianas") },
	{ wxT("Hard performance tracking"), wxT("Control estricto rendimiento") },
	{ wxT("Save preferences on exit"), wxT("Guardar configuraci髇 al salir") },

	{ wxT("Itinerary"), wxT("Trayecto") },
	{ wxT("Name :"), wxT("Nombre :") },
	{ wxT("From signal '%s'"), wxT("Desde se馻l '%s'") },
	{ wxT("Next itinerary :"), wxT("Siguiente trayecto") },

	{ wxT("Performance status"), wxT("Sumario configuraci髇") },
	{ wxT("Wrong destinations :"), wxT("Errores en el destino :") },
	{ wxT("Late trains :"), wxT("Trenes retrasados :") },
	{ wxT("Thrown switches :"), wxT("Errores al cambiar las agujas :") },
	{ wxT("Cleared signals :"), wxT("Errores al cambiar a verde las se馻les :") },
	{ wxT("Turned trains :"), wxT("Errores al invertir la direcci髇 :") },
	{ wxT("Trains stopped at sig.:"), wxT("Trenes parados en se馻les :") },
	{ wxT("Wrong platforms :"), wxT("Andenes err髇eos :") },
	{ wxT("Commands denied :"), wxT("Acci髇 denegada :") },
	{ wxT("Wrong stock assignments :"), wxT("Asignaci髇 de material err髇eo :") },

	{ wxT("Created by"), wxT("Creado por") },
	{ wxT("This is a game, and is not intended to\nbe used to actually control train traffic."), wxT("Esto es un juego, y no es su intencion \nel ser usado para el control real de tr醘ico ferroviario.") },
	{ wxT("This is free software, released under the\nGNU General Public License."), wxT(" Esto es un programa gratuito, distribuido bajo la licencia \nGNU General Public License.") },
	{ wxT("This is free software, released under the\nGNU General Public License.\nThe author declines any responsibility for any damage\nthat might occur from the use of this program."), wxT(" Esto es un programa gratuito, distribuido bajo la licencia \nGNU General Public License. El autor declina cualquier responsabilidad por cualquier da駉\nque pueda ocurrir por el uso de este programa.") },
	{ wxT("This is free software, released under the\nGNU General Public License Version 2.\nThe author declines any responsibility for any damage\nthat might occur from the use of this program."), wxT(" Esto es un programa gratuito, distribuido bajo la licencia \nGNU General Public License Version 2. \nEl autor declina cualquier responsabilidad por cualquier da駉\nque pueda ocurrir por el uso de este programa.") },

	{ wxT("Layout Errors"), wxT("Errores en el escen醨io") },
	{ wxT("These trains will be delayed on entry:"), wxT("Estos trenes entrar醤 con retraso:") },
	{ wxT("%s and %s both enter at %s on %s"), wxT("Y %s che %s entran en %s en %s") },
	{ wxT("These trains have unknown entry or exit points:"), wxT("Estos trenes tienen un punto de entrada o salida desconocido") },
	{ wxT("%s enters from '%s', exits at '%s'"), wxT("'%s' entra por '%s', sale por '%s'") },

	{ wxT("Some train has unknown entry/exit point!"), wxT("lgunos trenes tienen punto de entrada/salida desconocido!") },
	{ wxT("at"), wxT("en") },


	{ wxT("Start"), wxT("Iniciar") },
	{ wxT("Fast"), wxT("R醦ido") },
	{ wxT("Slow"), wxT("Lento") },
	{ wxT("Sched."), wxT("Horario") },

	//{ wxT("Simulation results"), wxT("Resultado de la simulaci髇") },
	//{ wxT("Time"), wxT("Hora") },
	//{ wxT("Total points"), wxT("Puntuaci髇 total") },
	//{ wxT("Total min. of delayed entry"), wxT("Minutos totales de retraso en la entrada") },
	//{ wxT("Total min. trains arrived late"), wxT("Minutos totales de retraso en la llegada") },
	//{ wxT("Total performance penalties"), wxT("Penalizaciones totales") },
	//{ wxT("Wrong destinatios"), wxT("Destinos err髇eos") },
	//{ wxT("Late trains"), wxT("Trenes retrasados") },
	//{ wxT("Wrong platforms"), wxT("Errores en el and閚") },
	//{ wxT("Commands denied"), wxT("Acciones denegadas") },
	//{ wxT("Trains waiting at signals"), wxT("Trenes esperando en se馻l") },
	//{ wxT("Thrown switches"), wxT("Errores al cambiar de agujas") },
	//{ wxT("Cleared signals"), wxT("Errores al cambiar a verde una se馻l") },
	//{ wxT("Wrong stock assignments"), wxT("Errores en la asignaci髇 de material") },
	//{ wxT("Reversed trains"), wxT("Errores al invertir direcci髇") },

{ wxT("仿真结果"), wxT("Resultado de la simulaci髇") },
	{ wxT("时间"), wxT("Hora") },
	{ wxT("总分"), wxT("Puntuaci髇 total") },
	{ wxT("延迟进入"), wxT("Minutos totales de retraso en la entrada") },
	{ wxT("总晚点时间"), wxT("Minutos totales de retraso en la llegada") },
	{ wxT("Total performance penalties"), wxT("Penalizaciones totales") },
	{ wxT("行进方向错误"), wxT("Destinos err髇eos") },
	{ wxT("晚点列车数"), wxT("Trenes retrasados") },
	{ wxT("站台错误"), wxT("Errores en el and閚") },
	{ wxT("拒绝命令"), wxT("Acciones denegadas") },
	{ wxT("等待信号灯时间"), wxT("Trenes esperando en se馻l") },
	{ wxT("股道封闭"), wxT("Errores al cambiar de agujas") },
	{ wxT("信号灯变绿"), wxT("Errores al cambiar a verde una se馻l") },
	{ wxT("错误分配列车"), wxT("Errores en la asignaci髇 de material") },
	{ wxT("列车反向"), wxT("Errores al invertir direcci髇") },



	{ wxT("Enters"), wxT("Entradas") },
	{ wxT("At"), wxT("En") },
	{ wxT("Exits"), wxT("Salidas") },
	{ wxT("Before"), wxT("Antes") },
	{ wxT("Delay"), wxT("Retraso entrada") },
	{ wxT("Late"), wxT("Retraso") },
	{ wxT("Status"), wxT("Estado") },

	{ wxT("Station"), wxT("Estaci髇") },
	{ wxT("Arrival"), wxT("Llegada") },
	{ wxT("Departure"), wxT("Salida") },
	{ wxT("Min.stop"), wxT("Minutos parada") },
	{ wxT("Plat."), wxT("And閚") },
	{ wxT("Runs on"), wxT("Circula el") },
	{ wxT("Notes"), wxT("Notas") },
	{ wxT("To&nbsp;&nbsp;"), wxT("De&nbsp;&nbsp;") },
	{ wxT("To&nbsp;&nbsp;&nbsp;"), wxT("De&nbsp;&nbsp;&nbsp;") },
	{ wxT("Runs on&nbsp;&nbsp;"), wxT("Circula el&nbsp;") },
	{ wxT("Runs&nbsp;on&nbsp;&nbsp;"), wxT("Circula&nbsp;el&nbsp;") },
	{ wxT("Minimum stop (sec)"), wxT("Parada m韓ima (sec)") },

	{ wxT("Territory"), wxT("Territorio") },

	{ wxT("Are you sure you want to restore\nthe simulation to its saved state?"), wxT("縀st?seguro que quiere restaurar\nla simulaci髇 a su estado inicial guardado?") },
	{ wxT("This will delete the current layout.\nAre you sure you want to continue?"), wxT("Esta acci髇 borrar?el escenario actual. \縎eguro que quiere continuar?") },
	{ wxT("Replay interrupted."), wxT("Reinicio interrumpido.") },


	{ wxT("/File/Open recent"), wxT("/Archivo/Abrir recientes") },

	{ wxT("Train %s waiting at %d,%d!"), wxT("Tren %s esperando en %d,%d!") },
	{ wxT("Train %s waiting at"), wxT("Tren %s esperando en") },
	{ wxT("You must assign train %s using stock from train %s!"), wxT("Debes reasignar el tren %s usando el material del tren %s!") },
	{ wxT("Train %s derailed at %s!"), wxT("Tren %s descarrilado en %s!") },
	{ wxT("Train %s derailed at %d,%d!"), wxT("Tren %s descarrilado en %d,%d!") },
	{ wxT("Train %s derailed!"), wxT("Tren %s descarrilado") },
	{ wxT("Train %s delayed at %s!"), wxT("Tren %s retrasado en %s!") },
	{ wxT("Cannot reverse direction. Path is busy."), wxT("Imposible invertir direcci髇. V韆 ocupada.") },
	{ wxT("This signal cannot be turned to green!"), wxT("sta se馻l no se puede cambiar a verde!") },
	{ wxT("You must wait for train to stop."), wxT("Debes esperar a que el tren se detenga.") },
	{ wxT("Reverse train direction?"), wxT("縄nvertir la direcci髇 del tren?") },
	{ wxT("Proceed to next station?"), wxT("緼vanzar a la siguiente estaci髇?") },

	{ wxT("Sorry, this feature is not available on this scenario."), wxT("Lo siento, esta funci髇 no est?disponible en este escenario.") },
	{ wxT("No station has distance information."), wxT("Ninguna estaci髇 tiene informaci髇 de la distancia.") },
	{ wxT("Train Schedule Graph"), wxT("Grafico hOrario") },

	{ wxT("Error reading"), wxT("Error al leer") },
	{ wxT("Final stop"), wxT("趌tima parada") },
	{ wxT("at"), wxT("en") },
	{ wxT("Next stop"), wxT("Siguiente parada") },
	{ wxT("ready"), wxT("listo") },
	{ wxT("Canceled - runs on"), wxT("Suprimido - circula el") },
	{ wxT("Shunting"), wxT("Maniobra") },
	{ wxT("to"), wxT("a") },
	{ wxT("Running. Dest."), wxT("Circulando. Dest.") },
	{ wxT("Stopped. ETD"), wxT("Parado. ETD") },
	{ wxT("Dest."), wxT("Dest.") },
	{ wxT("Delayed entry at"), wxT("Entrada retrasada en") },
	{ wxT("Waiting"), wxT("Esperando") },
	{ wxT("derailed"), wxT("descarrilado") },
	{ wxT("Arrived at"), wxT("Lleg?a") },
	{ wxT("instead of"), wxT("En vez de a") },
	{ wxT("Arrived"), wxT("Llegada") },
	{ wxT("min. late at"), wxT("min. de retraso en") },
	{ wxT("Arrived on time"), wxT("Llegada a tiempo") },
	{ wxT("stock for"), wxT("Asignar a") },

	{ wxT("Signal at"), wxT("Se馻l en") },
	{ wxT("not linked to any track"), wxT("no asociado a ning鷑 tramo") },
	{ wxT("Track at"), wxT("Tramo en") },
	{ wxT("not controlled by signal at"), wxT("no controlado por se馻l en") },
	{ wxT("linked to non-existant track at"), wxT("Asociado a un tramo inexistente") },
	{ wxT("not linked back to"), wxT("No reasociado a") },
	{ wxT("Switch at"), wxT("Cambio en") },
	{ wxT("linked to non-existant switch at"), wxT("Asociado a un cambio inexistente en") },
	{ wxT("not linked back to switch at"), wxT("no reasociado al cambio en") },
	{ wxT("Checking for errors in layout...\n"), wxT("Buscando errores en la ruta...\n") },
	{ wxT("nly like tracks can be linked."), wxT("S髄o tramos compatibles pueden ser asociados.") },
	{ wxT("Only horizontal or vertical tracks can be linked automatically."), wxT("S髄o tramos horizontales o verticales pueden ser asociados autom醫icamente. Para asociar otros tipos de tramos, usar \"propiedades\".") },
	{ wxT("Signals can only be linked to a track."), wxT("Una se馻l s髄o puede ser asociada a un tramo.") },
	{ wxT("Triggers can only be linked to a track."), wxT("Un pedal s髄o puede ser asociada a un tramo.") },
	{ wxT("Entry/Exit point can only be linked to a track."), wxT("Un punto de entrada/salida s髄o puede ser asociado a un tramo.") },

	{ wxT("speed"), wxT("veloc.") },
	{ wxT("length"), wxT("longit.") },
	{ wxT("Station"), wxT("Estaci髇") },
	{ wxT("entry/exit"), wxT("entrada/salida") },
	{ wxT("Signal"), wxT("Se馻l") },
	{ wxT("controls"), wxT("Controles") },

	{ wxT("Station of"), wxT("Estaci髇 de") },
	{ wxT("Platform"), wxT("And閚") },
	{ wxT("An itinerary by the same name already exists.\nDo you want to replace the old itinerary with the new one?"), wxT("Un itinerario con el mismo nombre ya existe. \縌uiere reemplazar el antiguo itinerario por este nuevo?") },

	{ wxT("Cannot create log file."), wxT("Imposible crear el archivo log.") },
	{ wxT("Cannot read log file."), wxT("Imposible leer el archivo log.") },
	{ wxT("Do you want to restart the simulation?"), wxT("縌uiere reiniciar la simulaci髇 desde el inicio?") },
	{ wxT("Simulation restarted."), wxT("Simulaci髇 reiniciada desde le inicio.") },

	{ wxT("Stop"), wxT("Parar") },
	{ wxT("Start"), wxT("Iniciar") },
	{ wxT("Restart"), wxT("Reiniciar") },
	{ wxT("Layout saved in file"), wxT("Trayecto guardado en le archivo") },
	{ wxT("Game status saved in file"), wxT("Estado del juego guardado en el archivo") },
	{ wxT("No schedule for this territory!"), wxT("orario inexistente para esta ruta!") },
	{ wxT("Question"), wxT("Pregunta") },
	{ wxT("Warning"), wxT("Atenci髇") },
	{ wxT("Attention"), wxT("Error") },
	{ wxT("Open File"), wxT("Abrir archivo") },
	{ wxT("Error"), wxT("Error") },
	{ wxT("Scenarios"), wxT("Escenarios") },
	{ wxT("Saved games"), wxT("Partida guardada") },
	{ wxT("All files (*.*)"), wxT("Todos los archivos (*.*)") },
	{ wxT("Icons"), wxT("Iconos") },
	{ wxT("Save File"), wxT("Guardar archivo") },
	{ wxT("Can't create file"), wxT("Imposible crear el archivo") },

	{ wxT("/_文件"), wxT("/_Archivo") },
	{ wxT("/File/_Open"), wxT("/Archivo/_Abrir") },
	{ wxT("/File/Open re_cent"), wxT("/Archivo/Abrir recientes") },
	{ wxT("/File/_Save Game"), wxT("/Archivo/_Guardar partida") },
	{ wxT("/File/_Restore"), wxT("/Archivo/_Restaurar") },
	{ wxT("/File/Save _As"), wxT("/Archivo/Guardar _Como") },
	{ wxT("/File/_Log"), wxT("/Archivo/Re_gistra") },
	{ wxT("/File/_Replay"), wxT("/Archivo/_Repetir") },
	{ wxT("/File/About..."), wxT("/Archivo/Informaci髇...") },
	{ wxT("/File/Quit"), wxT("/Archivo/Cerrar") },
	{ wxT("/Edit/_Edit"), wxT("/Editar/_Editar") },
	{ wxT("/Edit/_New Train"), wxT("/Editar/_Nuevo Tren") },
	{ wxT("/Edit/Itinerary"), wxT("/Editar/Itinerario") },
	{ wxT("/Edit/_Save Layout"), wxT("/Editar/_Guardar Ruta") },
	{ wxT("/Edit/_Preferences"), wxT("/Editar/_Preferencias") },
	{ wxT("/Edit/_New"), wxT("/Editar/_Nueva Ruta") },
	{ wxT("/Edit/_Info"), wxT("/Editar/_Informaci髇") },
	{ wxT("/_Run"), wxT("/_Simular") },
	{ wxT("/Run/_Start"), wxT("/Simular/_Iniciar") },
	{ wxT("/Run/View _Graph"), wxT("/Simular/Horario _Gr醘ico") },
	{ wxT("/Run/Restart"), wxT("/Simular/Reiniciar") },
	{ wxT("/Run/_Fast"), wxT("/Simular/_R醦ido") },
	{ wxT("/Run/S_low"), wxT("/Simular/_Lento") },
	{ wxT("/Run/S_tation schedule"), wxT("/Simular/Horario de _Estaciones") },
	{ wxT("/Run/Set sig. to green"), wxT("/Simular/Se馻les autom. a verde") },
	{ wxT("/Run/Performance"), wxT("/Simular/Configuraci髇") },

	{ wxT("This is a game, and is not intended to\nbe used to actually control train traffic."), wxT("Esto es un juego, y no es su intencion \nel ser usado para el control real de tr醘ico ferroviario.") },
	{ wxT("This is free software, released under the\nGNU General Public License Version 2."), wxT(" Esto es un programa gratuito, distribuido bajo la licencia \nGNU General Public License Version 2.") },
	{ wxT("The author declines any responsibility for any damage\nthat might occur from the use of this program."), wxT("El autor declina cualquier responsabilidad por cualquier da駉\nque pueda ocurrir por el uso de este programa.") },

	{ wxT("Assign rolling stock"), wxT("Asignar material rodante") },
	{ wxT("&Assign"), wxT("&Asignar") },
	{ wxT("S&hunt"), wxT("&Maniobra") },
	{ wxT("Sp&lit"), wxT("Desa&copla") },
	{ wxT("&Properties"), wxT("&Propiedades") },
	{ wxT("&Cancel"), wxT("&Cancelar") },

	{ wxT("Itinerary definition"), wxT("Definici髇 itinerario") },
	{ wxT("&Name"), wxT("Nombre") },
	{ wxT("From signal"), wxT("De la se馻l") },
	{ wxT("Ne&xt itinerary"), wxT("&Siguiente itinerario") },

	{ wxT("Performance"), wxT("Configuraci髇") },
	{ wxT("Count"), wxT("Frecuencia") },
	{ wxT("Points"), wxT("Puntos") },
	{ wxT("Total"), wxT("Total") },
	{ wxT("Wrong destinations"), wxT("Destinos err髇eos") },
	{ wxT("Late trains"), wxT("Trenes llegados con retraso") },
	{ wxT("Wrong platforms"), wxT("Errores de and閚") },
	{ wxT("Commands denied"), wxT("Acciones denegadas") },
	{ wxT("Trains stopped at signals"), wxT("Trenes parados en se馻l") },
	{ wxT("Turned trains"), wxT("Errores en la inversi髇 del tren") },
	{ wxT("Thrown switches"), wxT("Errores al cambiar de agujas") },
	{ wxT("Cleared signals"), wxT("Errores al cambiar a verde una se馻l") },
	{ wxT("Wrong stock assignments"), wxT("Errores al asignar el material") },


	{ wxT("Preferences"), wxT("Preferencias") },
	{ wxT("Short train info"), wxT("Informaci髇 tren reducida") },
	{ wxT("Status line at top of window"), wxT("L韓ea de estado arriba") },
	{ wxT("Alert sound on"), wxT("Se馻l de aviso encendida") },
	{ wxT("Alert on train entering layout"), wxT("Aviso sonoro al entrar un tren en el escenario") },
	{ wxT("View speed limits"), wxT("Mostrar los puntos de l韒ite de velocidad") },
	{ wxT("Automatically link signals"), wxT("Asociar autom醫icamente las se馻les") },
	{ wxT("Show grid"), wxT("Mostar cuadr韈ula") },
	{ wxT("View long blocks"), wxT("Mostrarl longitud tramos") },
	{ wxT("Show seconds on clock"), wxT("Mostrar los segundos en el reloj") },
	{ wxT("Traditional signals"), wxT("Se馻les tradicionales") },
	{ wxT("Strong performance checking"), wxT("Control m醩 restrictivo de error") },
	{ wxT("Show linked objects in editor"), wxT("Mostrar objetos asociados en el editor") },
	{ wxT("Check the desired options:"), wxT("Seleccione la opci髇 deseada:") },
	{ wxT("&Late Graph\tCtrl-L"), wxT("Retraso Acumulado\tCtrl-L") },
	{ wxT("&Platform Graph"), wxT("Ocupaci髇 de los Andenes ") },

	{ wxT("Day selection"), wxT("Seleccione d韆") },
	{ wxT("&Continue"), wxT("&Continuar") },

	{ wxT("Signal at coordinates :"), wxT("Se馻l en coordenadas :") },
	{ wxT("Signal name :"), wxT("Nombre se馻l") },
	{ wxT("Linked to east :"), wxT("Asociado al este :") },
	{ wxT("Linked to west :"), wxT("Asociado al oeste :") },
	{ wxT("Signal is always red"), wxT("Se馻l siempre en rojo") },
	{ wxT("Signal has square frame"), wxT("Se馻l de forma cuadrada") },
	{ wxT("No penalty for train stopping at this signal"), wxT("No penalizar por trenes parados en esta se馻l") },

	{ wxT("S&tation schedule\tF6"), wxT("Horario de estaci髇\tF6") },
	{ wxT("Station schedule"), wxT("Horario de estaci髇") },
	{ wxT("S&tation schedule\tF5"), wxT("Horario de es&taci髇\tF5") },
	{ wxT("&Schedule for station :"), wxT("&Horario para la estaci髇 de :") },
	{ wxT("&Ignore platform number"), wxT("&Ignorar and閚 de llegada") },
	{ wxT("&Close"), wxT("&Cerrar") },
	{ wxT("&Print"), wxT("&Imprimir") },

	{ wxT("Select Itinerary\tCtrl-I"), wxT("Seleccion Itinerario\tCtrl-I") },
	{ wxT("Select Itinerary"), wxT("Selecciona Itinerario") },
	{ wxT("Select itinerary"), wxT("Selecciona itinerario") },

	{ wxT("Layout Information"), wxT("Informaci髇 del trayecto") },

	{ wxT("Track Properties"), wxT("Propiedades del tramo") },
	{ wxT("Track Length (m) :"), wxT("Longitud del tramo (m) :") },
	{ wxT("Station name :"), wxT("Nombre Estaci髇 :") },
	{ wxT("Speed(s) :"), wxT("Velocidad") },

	{ wxT("Train Properties"), wxT("Propiedades del tren") },
	{ wxT("Train name"), wxT("Nombre del tren") },
	{ wxT("Train type"), wxT("Tipo de tren") },
	{ wxT("Entry point"), wxT("Punto de entrada") },
	{ wxT("Entry time (hh:mm)"), wxT("Horario de entrada (hh:mm)") },
	{ wxT("Exit point"), wxT("Punto de salida") },
	{ wxT("Exit time (hh:mm)"), wxT("Horario de salida (hh:mm)") },
	{ wxT("Wait arrival of train"), wxT("Esperar llegada del tren") },
	{ wxT("Stock for train"), wxT("Material para el tren") },
	{ wxT("Runs on"), wxT("Circula el") },
	{ wxT("Train length :"), wxT("Longitud del tren :") },

	{ wxT("Trigger Properties"), wxT("Propiedad Pedal") },
	{ wxT("Trigger at coordinates :"), wxT("Pedal a las coordenadas :") },
	{ wxT("Action\n(e.g. \"click 12,4\")"), wxT("Acci髇\n(es. \"click 12,4\")") },
	{ wxT("Linked to track at :"), wxT("Asociado al tramo en :") },
	{ wxT("Probabilities :"), wxT("Probabilidades") },
	{ wxT("Action:             ('@' in action = name of triggering train)"), wxT("Acci髇: ('@' en acci髇 = nombre del tren activante)") },
	{ wxT("Linked to track at coord :"), wxT("Asignar al tramo en coord. :") },
	{ wxT("Probabilities for action :"), wxT("Probabilidad para la acci髇 :") },

	{ wxT("cannot load"), wxT("Imposible cargar") },
	{ wxT("Destination"), wxT("Destino") },
	{ wxT("Assigning stock of train %s arrived at station %s"), wxT("Asignar material del tren %s llegado a %s") },
	{ wxT("From signal '%s'"), wxT("Desde la se馻l '%s'") },

	{ wxT("&File"), wxT("&Archivo") },
	{ wxT("&Open...\tCtrl-O"), wxT("&Abrir...\tCtrl-O") },
	{ wxT("&Save Game..."), wxT("&Guardar Partida...") },
	{ wxT("&Restore..."), wxT("&Restaurar simulaci髇...") },
	{ wxT("Page set&up"), wxT("Config&urar P醙ina impresa") },
	{ wxT("Pre&view"), wxT("Pre&ver la impresi髇") },
	{ wxT("&Print"), wxT("Imprimir&pa") },
	{ wxT("&Edit"), wxT("&Editar") },
	{ wxT("&Itinerary"), wxT("&Itinerario") },
	{ wxT("&Save Layout"), wxT("&Guardar Trayecto") },
	{ wxT("&Preferences..."), wxT("&Preferencias") },
	{ wxT("Exit\tAlt-F4"), wxT("Salir\tAlt-F4") },
	{ wxT("Ne&w"), wxT("&Nueva ruta") },
	{ wxT("In&fo"), wxT("&Informaci髇") },
	{ wxT("&Run"), wxT("&Simular") },
	{ wxT("&Start\tCtrl-S"), wxT("&Iniciar\tCtrl-S") },
	{ wxT("&Graph\tCtrl-G"), wxT("Horario &Gr醘ico\tCtrl-G") },
	{ wxT("&Restart..."), wxT("&Reiniciar...") },
	{ wxT("&Fast\tCtrl-X"), wxT("&R醦ido\tCtrl-X") },
	{ wxT("&Slow\tCtrl-Z"), wxT("&Lento\tCtrl-Z") },
	{ wxT("Set sig. to green"), wxT("Se馻les autom. a verde") },
	{ wxT("&Performance"), wxT("Configuraci髇") },

	{ wxT("&View"), wxT("&Mostrar") },
	{ wxT("Show layout\tF3"), wxT("Mostrar trayecto\tF3") },
	{ wxT("Show schedule\tF4"), wxT("Mostrar horario\tF4") },
	{ wxT("Timetable in split window"), wxT("Horario en recuadro") },
	{ wxT("Timetable in tabbed window"), wxT("Horario en panel") },
	{ wxT("Timetable in separate window"), wxT("Horario en ventana separada") },
	{ wxT("Tool bar"), wxT("Barra de herramientas") },
	{ wxT("Status bar"), wxT("L韓ea de estado") },

	{ wxT("Welcome"), wxT("Bienvenido") },
	{ wxT("Welcome\tF1"), wxT("Bienvenido\tF1") },
	{ wxT("&Help"), wxT("&Ayuda") },

	{ wxT("Tools"), wxT("Herramientas") },
	{ wxT("Graph"), wxT("Gr醘ico") },
	{ wxT("Layout"), wxT("Trayecto") },

	{ wxT("You recently played the following simulations:<br><br>"), wxT("Rutas simuladas recientemente:<br><br>") },
	{ wxT("Clear"), wxT("Limpiar") },

	{ wxT("Remove all alerts from this list?"), wxT("縀liminar todas las alertas de esta lista?") },
	{ wxT("Show Canceled"), wxT("Mostrar suprimidos") },
	{ wxT("Track First Train"), wxT("Seguir Primer Tren Activo") },
	{ wxT("Track Last Train"), wxT("Seguir 趌timo Tren Activo") },
	{ wxT("Show/hide canceled trains from list"), wxT("Mostrar/esconder de la lista los trenes que no circulan") },
	{ wxT("Automatically show first active train in list"), wxT("Mostrar automaticamente en la lista el primer tren activo") },
	{ wxT("Automatically show last active train in list"), wxT("Mostrar automaticamente en la lista el 鷏timo tren activo") },

	{ wxT("Stations &List"), wxT("&Lista Estaciones") },
	{ wxT("Stations List"), wxT("Lista Estaciones") },
	{ wxT("Stations and Entry/Exit Points"), wxT("Estaci髇 y Punto de Entrada/Salida") },
	{ wxT("Station Name"), wxT("Nombre estaci髇") },
	{ wxT("Coordinates"), wxT("Coordenadas") },
	{ wxT("Entry/Exit"), wxT("Entrada/Salida") },

	{ wxT("Zoom in"), wxT("Acercar") },
	{ wxT("Zoom out"), wxT("Alejar") },
	{ wxT("Coord bars"), wxT("Coordenadas") },

	{ wxT("Delete Column"), wxT("Borrar Columna") },
	{ wxT("Delete Columns..."), wxT("Borrar Columnas...") },
	{ wxT("Insert Column"), wxT("Insertar Columna") },
	{ wxT("Insert Columns..."), wxT("Inserar columnas...") },
	{ wxT("Delete Row"), wxT("Borrar Fila") },
	{ wxT("Delete Rows..."), wxT("Borrar Filas...") },
	{ wxT("Insert Row"), wxT("Insertar Fila") },
	{ wxT("Insert Rows..."), wxT("Insertar Filas...") },
	{ wxT("Delete"), wxT("Borrar ") },
	{ wxT("Day"), wxT("D韆 ") },
	{ wxT("Assign&+Shunt"), wxT("Asigna&+Maniobra") },
	{ wxT("&Reverse+Assign"), wxT("Invie&rte+Asigna") },
	{ 0 }
};

