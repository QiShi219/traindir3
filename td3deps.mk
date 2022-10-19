$(OBJDIR)Canvas.o: Canvas.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) Canvas.cpp

$(OBJDIR)Shape.o: Shape.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) Shape.cpp

$(OBJDIR)html.o: html.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) html.cpp

$(OBJDIR)http.o: http.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) http.cpp

$(OBJDIR)AlertList.o: AlertList.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) AlertList.cpp

$(OBJDIR)Alerts.o: Alerts.cpp Alerts.h
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) Alerts.cpp

$(OBJDIR)TrainInfoList.o: TrainInfoList.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) TrainInfoList.cpp

$(OBJDIR)AssignDialog.o: AssignDialog.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) AssignDialog.cpp

$(OBJDIR)DaysDialog.o: DaysDialog.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) DaysDialog.cpp

$(OBJDIR)ItineraryDialog.o: ItineraryDialog.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) ItineraryDialog.cpp

$(OBJDIR)ConfigDialog.o: ConfigDialog.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) ConfigDialog.cpp

$(OBJDIR)OptionsDialog.o: OptionsDialog.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) OptionsDialog.cpp

$(OBJDIR)SignalDialog.o: SignalDialog.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) SignalDialog.cpp

$(OBJDIR)TrackDialog.o: TrackDialog.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) TrackDialog.cpp

$(OBJDIR)TriggerDialog.o: TriggerDialog.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) TriggerDialog.cpp

$(OBJDIR)StationInfoDialog.o: StationInfoDialog.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) StationInfoDialog.cpp

$(OBJDIR)TrainInfoDialog.o: TrainInfoDialog.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) TrainInfoDialog.cpp

$(OBJDIR)loadsave.o: loadsave.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) loadsave.cpp

$(OBJDIR)Main.o: Main.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) Main.cpp

$(OBJDIR)MainFrm.o: MainFrm.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) MainFrm.cpp

$(OBJDIR)run.o: run.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) run.cpp

$(OBJDIR)TConfig.o: TConfig.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) TConfig.cpp

$(OBJDIR)TDFile.o: TDFile.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) TDFile.cpp

$(OBJDIR)ReportBase.o: ReportBase.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) ReportBase.cpp

$(OBJDIR)GraphView.o: GraphView.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) GraphView.cpp

$(OBJDIR)LateGraphView.o: LateGraphView.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) LateGraphView.cpp

$(OBJDIR)HtmlView.o: HtmlView.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) HtmlView.cpp

$(OBJDIR)ItineraryView.o: ItineraryView.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) ItineraryView.cpp

$(OBJDIR)ToolsView.o: ToolsView.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) ToolsView.cpp

$(OBJDIR)TimeTable.o: TimeTable.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) TimeTable.cpp

$(OBJDIR)TimeTblView.o: TimeTblView.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) TimeTblView.cpp

$(OBJDIR)NotebookMgr.o: NotebookMgr.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) NotebookMgr.cpp

$(OBJDIR)track.o: track.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) track.cpp

$(OBJDIR)Train.o: Train.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) Train.cpp

$(OBJDIR)MotivePower.o: MotivePower.cpp MotivePower.h
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) MotivePower.cpp

$(OBJDIR)trsim.o: trsim.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) trsim.cpp

$(OBJDIR)Itinerary.o: Itinerary.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) Itinerary.cpp

$(OBJDIR)Localize.o: Localize.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) Localize.cpp

$(OBJDIR)TSignal.o: TSignal.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) TSignal.cpp

$(OBJDIR)tdscript.o: tdscript.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) tdscript.cpp

$(OBJDIR)ScenarioInfoDialog.o: ScenarioInfoDialog.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) ScenarioInfoDialog.cpp

$(OBJDIR)TrackScriptDialog.o: TrackScriptDialog.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) TrackScriptDialog.cpp

$(OBJDIR)FontManager.o: FontManager.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) FontManager.cpp


$(OBJDIR)PlatformGraph.o: PlatformGraph.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) PlatformGraph.cpp

$(OBJDIR)Puzzles.o: Puzzles.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) Puzzles.cpp

$(OBJDIR)GTFS.o: GTFS.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) GTFS.cpp

$(OBJDIR)CSV.o: CSV.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) CSV.cpp


$(OBJDIR)Client.o: Client.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) Client.cpp

$(OBJDIR)Options.o: Options.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) Options.cpp

$(OBJDIR)ClientConnection.o: ClientConnection.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) ClientConnection.cpp

$(OBJDIR)AppMainLoop.o: AppMainLoop.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) AppMainLoop.cpp

$(OBJDIR)WebServices.o: WebServices.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) WebServices.cpp

$(OBJDIR)WSGenTT.o: WSGenTT.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) WSGenTT.cpp

$(OBJDIR)Server.o: Server.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) Server.cpp

$(OBJDIR)StringBuilder.o: StringBuilder.cpp StringBuilder.h
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) StringBuilder.cpp

$(OBJDIR)TVector.o: TVector.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) TVector.cpp

$(OBJDIR)HostLock.o: HostLock.cpp HostLock.h
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) HostLock.cpp

$(OBJDIR)HostThread.o: HostThread.cpp HostThread.h
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) HostThread.cpp

$(OBJDIR)SwitchBoard.o: SwitchBoard.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) SwitchBoard.cpp

$(OBJDIR)SwitchboardNameDialog.o: SwitchboardNameDialog.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) SwitchboardNameDialog.cpp

$(OBJDIR)SwitchboardView.o: SwitchboardView.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) SwitchboardView.cpp

$(OBJDIR)SkinColorsDialog.o: SkinColorsDialog.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) SkinColorsDialog.cpp

$(OBJDIR)mongoose.o: mongoose.c
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) mongoose.c

$(OBJDIR)italiano.o: italiano.cpp
	$(CXXC) -c -o $@ $(TD3_CXXFLAGS) italiano.cpp

