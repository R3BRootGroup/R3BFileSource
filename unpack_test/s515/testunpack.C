/**
 **  Macro to run the offline for all the detectors simultaneously
 **
 **  The unpacker is at $UCESB_DIR/../upexps/202104_s515
 **
 **  This macro generates a root file with all the data at mapped level using
 **  a lmd file as input
 **
 **  Author: Jose Luis <j.l.rodriguez.sanchez@udc.es>
 **  @since May 7th, 2023
 **
 **/

typedef struct EXT_STR_h101_t
{
    EXT_STR_h101_unpack_t unpack;
    EXT_STR_h101_TPAT_t unpacktpat;
    EXT_STR_h101_TRLO_onion_t trloscaler;

    EXT_STR_h101_SCI2_t s2;
    EXT_STR_h101_MUSIC_onion_t music;
    EXT_STR_h101_AMS_onion_t ams;
    EXT_STR_h101_CALIFA_t califa;
    EXT_STR_h101_LOS_t los;
    EXT_STR_h101_FIBTEN_onion_t fiber10;
    EXT_STR_h101_FIBELEVEN_onion_t fiber11;
    EXT_STR_h101_FIBTWELVE_onion_t fiber12;
    EXT_STR_h101_FIBTHIRTEEN_onion_t fiber13;
    EXT_STR_h101_FIB_onion_t spmtfiber;
    EXT_STR_h101_TOFD_onion_t tofd;
    EXT_STR_h101_PSP_onion_t psp;
    EXT_STR_h101_raw_nnp_tamex_onion_t neuland;

    EXT_STR_h101_SOFMWPC_onion_t mwpc;

    EXT_STR_h101_WRMASTER_t wrmaster;
    EXT_STR_h101_WRS2_t wrs2;
    EXT_STR_h101_WRLOS_t wrlos;
    EXT_STR_h101_TIMESTAMP_PSPX_t wrpspx;
    EXT_STR_h101_WRMUSIC_t wrmusic;
    EXT_STR_h101_WRCALIFA_t wrcalifa;
    EXT_STR_h101_WRNEULAND_t wrneuland;
} EXT_STR_h101;

void testunpack(const Int_t fRunId = 503, const Int_t nev = -1, const Int_t fExpId = 515)
{
    TString cRunId = Form("%04d", fRunId);
    TString cExpId = Form("%03d", fExpId);

    FairLogger::GetLogger()->SetLogScreenLevel("info");
    FairLogger::GetLogger()->SetColoredLog(true);

    TStopwatch timer;

    // File names and paths -----------------------------
    const TString ntuple_options = "RAW";
    const TString workDirectory = getenv("VMCWORKDIR");
    const TString ucesb_dir = getenv("UCESB_DIR");
    TString filename, outputFilename, upexps_dir, ucesb_path;

    // Input file
    filename = workDirectory + "/R3BFileSource/lmds/s515/main" + cRunId + "*.lmd";
    filename.ReplaceAll("//", "/");

    // Output file
    outputFilename = "s515_" + cRunId + "_map.root";
    outputFilename.ReplaceAll("//", "/");

    // UPEXPS path
    upexps_dir = ucesb_dir + "/../upexps";
    ucesb_path = upexps_dir + "/202104_s515/202104_s515 --allow-errors --input-buffer=70Mi";
    ucesb_path.ReplaceAll("//", "/");

    // Setup: Selection of detectors ------------------------
    // --- FRS
    // --------------------------------------------------------------------------
    Bool_t fFrsSci = true; // Start: Plastic scintillators at FRS
    // --- R3B standard
    // -----------------------------------------------------------------
    Bool_t fPsp = true;     // Psp: Silicon detectors for tracking
    Bool_t fLos = true;     // Los scintillator for R3B experiments
    Bool_t fAms = true;     // AMS tracking detectors
    Bool_t fCalifa = true;  // Califa calorimeter
    Bool_t fMusic = true;   // R3B-Music: Ionization chamber for charge-Z before GLAD
    Bool_t fFiber10 = true; // Fiber10 behind GLAD
    Bool_t fFiber11 = true; // Fiber11 behind GLAD
    Bool_t fFiber12 = true; // Fiber12 behind GLAD
    Bool_t fFiber13 = true; // Fiber13 behind GLAD
    Bool_t fTofD = true;    // ToF-Wall for time-of-flight of fragments behind GLAD
    Bool_t fNeuland = true; // NeuLAND for neutrons behind GLAD
    // --- Sofia
    // ------------------------------------------------------------------------
    Bool_t fMwpc0 = true; // MWPC0 for tracking at Cave-C entrance

    // Create online run ------------------------------------
    R3BEventHeader* EvntHeader = new R3BEventHeader();
    EvntHeader->SetExpId(fExpId);
    FairRunOnline* run = new FairRunOnline();
    run->SetEventHeader(EvntHeader);
    run->SetRunId(fRunId);
    run->SetSink(new FairRootFileSink(outputFilename));

    // Create source using ucesb for input ------------------
    EXT_STR_h101 ucesb_struct;

    R3BUcesbSource* source =
        new R3BUcesbSource(filename, ntuple_options, ucesb_path, &ucesb_struct, sizeof(ucesb_struct));
    source->SetMaxEvents(nev);

    // Add readers ------------------------------------------
    source->AddReader(new R3BUnpackReader(&ucesb_struct.unpack, offsetof(EXT_STR_h101, unpack)));

    auto trloiitpat_reader = new R3BTrloiiTpatReader(&ucesb_struct.unpacktpat, offsetof(EXT_STR_h101, unpacktpat));
    source->AddReader(trloiitpat_reader);

    source->AddReader(new R3BWhiterabbitMasterReader(
        (EXT_STR_h101_WRMASTER*)&ucesb_struct.wrmaster, offsetof(EXT_STR_h101, wrmaster), 0x1000));

    source->AddReader(new R3BTrloiiScalerReader((EXT_STR_h101_TRLO_onion*)&ucesb_struct.trloscaler,
                                                offsetof(EXT_STR_h101, trloscaler)));

    if (fFrsSci)
    {
        auto unpackWRS2 =
            new R3BWhiterabbitS2Reader((EXT_STR_h101_WRS2*)&ucesb_struct.wrs2, offsetof(EXT_STR_h101, wrs2), 0x200);
        auto unpacks2 = new R3BSci2Reader(&ucesb_struct.s2, offsetof(EXT_STR_h101_t, s2));
        source->AddReader(unpackWRS2);
        source->AddReader(unpacks2);
    }

    if (fLos)
    {
        auto unpackWRlos = new R3BWhiterabbitLosReader(
            (EXT_STR_h101_WRLOS*)&ucesb_struct.wrlos, offsetof(EXT_STR_h101, wrlos), 0x1100);
        source->AddReader(unpackWRlos);
        auto unpacklos = new R3BLosReader((EXT_STR_h101_LOS_t*)&ucesb_struct.los, offsetof(EXT_STR_h101, los));
        source->AddReader(unpacklos);
    }

    if (fPsp)
    {
        auto unpackWRpsp = new R3BWhiterabbitPspReader(
            (EXT_STR_h101_TIMESTAMP_PSPX*)&ucesb_struct.wrpspx, offsetof(EXT_STR_h101, wrpspx), 0xc00);
        auto unpackpsp = new R3BPspxReader((EXT_STR_h101_PSP*)&ucesb_struct.psp, offsetof(EXT_STR_h101, psp));
        source->AddReader(unpackWRpsp);
        source->AddReader(unpackpsp);
    }

    if (fAms)
    {
        auto unpackams = new R3BAmsReader((EXT_STR_h101_AMS_onion*)&ucesb_struct.ams, offsetof(EXT_STR_h101, ams));
        source->AddReader(unpackams);
    }

    if (fCalifa)
    {
        auto unpackcalifa =
            new R3BCalifaFebexReader((EXT_STR_h101_CALIFA*)&ucesb_struct.califa, offsetof(EXT_STR_h101, califa));
        auto unpackWRCalifa = new R3BWhiterabbitCalifaReader(
            (EXT_STR_h101_WRCALIFA*)&ucesb_struct.wrcalifa, offsetof(EXT_STR_h101, wrcalifa), 0xa00, 0xb00);
        source->AddReader(unpackcalifa);
        source->AddReader(unpackWRCalifa);
    }

    if (fMwpc0)
    {
        auto unpackmwpc = new R3BMwpcReader((EXT_STR_h101_SOFMWPC_t*)&ucesb_struct.mwpc, offsetof(EXT_STR_h101, mwpc));
        unpackmwpc->SetMaxNbDet(1);
        source->AddReader(unpackmwpc);
    }

    if (fMusic)
    {
        auto unpackWRmusic = new R3BWhiterabbitMusicReader(
            (EXT_STR_h101_WRMUSIC*)&ucesb_struct.wrmusic, offsetof(EXT_STR_h101, wrmusic), 0xe00);
        auto unpackmusic =
            new R3BMusicReader((EXT_STR_h101_MUSIC_t*)&ucesb_struct.music, offsetof(EXT_STR_h101, music));
        source->AddReader(unpackWRmusic);
        source->AddReader(unpackmusic);
    }

    if (fFiber10)
    {
        auto unpackfiber10 = new R3BFiberReader(
            "Fi10", (EXT_STR_h101_FIBTEN_onion*)&ucesb_struct.fiber10, offsetof(EXT_STR_h101_t, fiber10), 2, 256, 2);
        source->AddReader(unpackfiber10);
    }

    if (fFiber11)
    {
        auto unpackfiber11 = new R3BFiberReader(
            "Fi11", (EXT_STR_h101_FIBELEVEN_onion*)&ucesb_struct.fiber11, offsetof(EXT_STR_h101_t, fiber11), 2, 256, 2);
        source->AddReader(unpackfiber11);
    }

    if (fFiber12)
    {
        auto unpackfiber12 = new R3BFiberReader(
            "Fi12", (EXT_STR_h101_FIBTWELVE_onion*)&ucesb_struct.fiber12, offsetof(EXT_STR_h101_t, fiber12), 2, 256, 2);
        source->AddReader(unpackfiber12);
    }

    if (fFiber13)
    {
        auto unpackfiber13 = new R3BFiberReader("Fi13",
                                                (EXT_STR_h101_FIBTHIRTEEN_onion*)&ucesb_struct.fiber13,
                                                offsetof(EXT_STR_h101_t, fiber13),
                                                2,
                                                256,
                                                2);
        source->AddReader(unpackfiber13);
    }

    if (fFiber10 || fFiber11 || fFiber12 || fFiber13)
    {
        source->AddReader(new R3BBunchedFiberSPMTTrigReader((EXT_STR_h101_FIB*)&ucesb_struct.spmtfiber,
                                                            offsetof(EXT_STR_h101_t, spmtfiber)));
    }

    if (fTofD)
    {
        auto unpacktofd = new R3BTofdReader((EXT_STR_h101_TOFD_onion*)&ucesb_struct.tofd, offsetof(EXT_STR_h101, tofd));
        source->AddReader(unpacktofd);
    }

    if (fNeuland)
    {
        auto unpackneuland = new R3BNeulandTamexReader((EXT_STR_h101_raw_nnp_tamex_onion*)&ucesb_struct.neuland,
                                                       offsetof(EXT_STR_h101, neuland));
        source->AddReader(unpackneuland);
    }
    run->SetSource(source);

    // Initialize -------------------------------------------
    timer.Start();
    run->Init();

    // Run --------------------------------------------------
    run->Run((nev < 0) ? nev : 0, (nev < 0) ? 0 : nev);

    // Finish -----------------------------------------------
    timer.Stop();
    Double_t rtime = timer.RealTime();
    Double_t ctime = timer.CpuTime();
    std::cout << std::endl << std::endl;
    std::cout << "Output file is " << outputFilename << std::endl;
    std::cout << "Real time " << rtime / 60. << " min, CPU time " << ctime / 60. << " min" << std::endl << std::endl;
    std::cout << "Macro finished successfully." << std::endl;
}
