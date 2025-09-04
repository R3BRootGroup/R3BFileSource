/**
 **  Macro to run the data unpacking for all the detectors simultaneously
 **
 **  This macro generates a root file with all the data at mapped level using
 **  a lmd file as input
 **
 **  Author: Jose Luis <j.l.rodriguez.sanchez@udc.es>
 **  @since April 13th, 2025
 **
 **  const Int_t nev = -1; number of events to read, -1 - until CTRL+C
 **  Select experiment ID: 249
 **
 **  After defining the input file (filename), execute the macro
 **  >> root -l 'unpack_data("file_including_path")'
 **  example: >> root -l 'unpack_data("./lmd_stitched/main0150*.lmd")'
 **  or
 **  >> root -l 'unpack_data("file_including_path",runID)'
 **  where runID corresponds to the run number, for example:
 **  >> root -l 'unpack_data("./lmd_stitched/main0150*.lmd",150)'
 **
 **  example of "file_including_path"="/lustre/r3b/202506_g249/lmd_stitched/main0150*.lmd"
 **
 **/

typedef struct EXT_STR_h101_t
{
    EXT_STR_h101_unpack_t unpack;
    EXT_STR_h101_TPAT_t tpat;
    EXT_STR_h101_TRLO_onion_t trloscaler;

    EXT_STR_h101_FRSSCI_onion_t frssci;
    EXT_STR_h101_SOFMWPC_onion_t mwpc;
    EXT_STR_h101_LOS_onion_t los;
    EXT_STR_h101_ROLU_onion_t rolu;

    EXT_STR_h101_FOOT_onion_t foot;
    EXT_STR_h101_CALIFA_t califa;
    EXT_STR_h101_MOSAIC202506_onion_t mosaic;

    EXT_STR_h101_FIBEO_onion_t fib30; // Fib 30
    EXT_STR_h101_FIBEI_onion_t fib31; // Fib 31
    EXT_STR_h101_FIBEZ_onion_t fib32; // Fib 32
    EXT_STR_h101_FIBEE_onion_t fib33; // Fib 33
    EXT_STR_h101_TOFD_onion_t tofd;
    EXT_STR_h101_RPC_onion_t rpc;
    EXT_STR_h101_raw_nnp_tamex_onion_t neuland;

    EXT_STR_h101_WRMASTER_t wrmaster;
    EXT_STR_h101_WRS2_t wrs2;
    EXT_STR_h101_WRNEULAND_t wrneuland;
    EXT_STR_h101_WRFOOT_onion_t wrfoot;
} EXT_STR_h101;

void testunpack(const Int_t fRunId = 503, const Int_t nev = -1)
{
    const Int_t fExpId = 249;

    TString cRunId = Form("%04d", fRunId);
    TString cExpId = Form("%03d", fExpId);

    FairLogger::GetLogger()->SetLogScreenLevel("info");
    FairLogger::GetLogger()->SetColoredLog(true);

    TStopwatch timer;

    const TString workDirectory = getenv("VMCWORKDIR");
    TString filename = workDirectory + "/R3BFileSource/lmds/s515/main" + cRunId + "*.lmd";
    filename.ReplaceAll("//", "/");

    // file names and paths  -----------------------------------
    TString ntuple_options = "RAW";

    // Path to UPEXPS   ---------------------------------------
    const TString upexps_dir = gSystem->Getenv("UCESB_DIR");
    if (upexps_dir == "")
    {
        std::cout << "UPEXPS_DIR is not set, load the configuration file on your PC > source conf.sh" << std::endl;
        gApplication->Terminate();
    }
    std::cout << "UCESB_DIR = " << upexps_dir << std::endl;

    TString outputFilename = "g249_map_offline_run" + cRunId + ".root";
    outputFilename.ReplaceAll("//", "/");

    TString ucesb_path = upexps_dir + "/empty/empty --allow-errors --input-buffer=600Mi";
    ucesb_path.ReplaceAll("//", "/");

    // Setup: Selection of detectors
    UShort_t fNumSci = 2;
    Bool_t fFrsSci = true; // Start: Plastic scintillators at FRS
    // --- R3B standard -------------------------------------------
    Bool_t fMwpc0 = false;  // MWPC0 for tracking at entrance of Cave-C (NO used)
    Bool_t fLos = true;     // LOS detectors
    Bool_t fRolu = false;   // Rolu detector
    Bool_t fFoot = true;    // FOOT tracking detectors
    Bool_t fAlpide = true;  // ALIPDE tracking detectors
    Bool_t fCalifa = true;  // Califa calorimeter
    Bool_t fFib30 = true;   // Fiber 30 for fragment tracking behind GLAD
    Bool_t fFib31 = true;   // Fiber 31 for fragment tracking behind GLAD
    Bool_t fFib32 = true;   // Fiber 32 for fragment tracking behind GLAD
    Bool_t fFib33 = true;   // Fiber 33 for fragment tracking behind GLAD
    Bool_t fTofd = true;    // TofD detector
    Bool_t fRpc = true;     // RPC detector
    Bool_t fNeuland = true; // Neuland detector

    // Create source using ucesb for input
    // ----------------------------------------
    EXT_STR_h101 ucesb_struct;

    // Create online run
    // ----------------------------------------------------------
    auto* run = new FairRunOnline();
    auto* EvntHeader = new R3BEventHeader();
    EvntHeader->SetExpId(fExpId);
    run->SetEventHeader(EvntHeader);
    run->SetRunId(fRunId);
    run->SetSink(new FairRootFileSink(outputFilename));

    auto* source = new R3BUcesbSource(filename, ntuple_options, ucesb_path, &ucesb_struct, sizeof(ucesb_struct));
    source->SetMaxEvents(nev);

    // Add readers
    // ----------------------------------------------------------------
    source->AddReader(new R3BUnpackReader(&ucesb_struct.unpack, offsetof(EXT_STR_h101, unpack)));

    auto* trloii = new R3BTrloiiTpatReader(&ucesb_struct.tpat, offsetof(EXT_STR_h101, tpat));
    // trloii->SetTpatRange(1, 10);
    source->AddReader(trloii);

    source->AddReader(new R3BWhiterabbitMasterReader(
        (EXT_STR_h101_WRMASTER*)&ucesb_struct.wrmaster, offsetof(EXT_STR_h101, wrmaster), 0x1000));

    source->AddReader(new R3BTrloiiScalerReader((EXT_STR_h101_TRLO_onion*)&ucesb_struct.trloscaler,
                                                offsetof(EXT_STR_h101, trloscaler)));

    if (fFrsSci)
    {
        source->AddReader(
            new R3BFrsSciReader((EXT_STR_h101_FRSSCI*)&ucesb_struct.frssci, offsetof(EXT_STR_h101_t, frssci), fNumSci));
        source->AddReader(
            new R3BWhiterabbitS2Reader((EXT_STR_h101_WRS2*)&ucesb_struct.wrs2, offsetof(EXT_STR_h101, wrs2), 0x200));
    }

    if (fLos)
    {
        source->AddReader(new R3BLosReader((EXT_STR_h101_LOS*)&ucesb_struct.los, offsetof(EXT_STR_h101, los)));
    }

    if (fFoot)
    {
        auto unpackfoot =
            new R3BFootSiReader((EXT_STR_h101_FOOT_onion*)&ucesb_struct.foot, offsetof(EXT_STR_h101, foot));
        std::vector<int> footIds = { 2, 13, 4, 11, 7, 6, 9, 12, 1, 3, 5, 8, 10, 14, 15, 16 };
        unpackfoot->SetMapping(footIds);
        source->AddReader(unpackfoot);
    }

    if (fCalifa)
    {
        source->AddReader(
            new R3BCalifaFebexReader((EXT_STR_h101_CALIFA*)&ucesb_struct.califa, offsetof(EXT_STR_h101, califa)));
    }

    if (fAlpide)
    {
        auto mosaic =
            new R3BMosaicReader((EXT_STR_h101_MOSAIC202506_onion*)&ucesb_struct.mosaic, offsetof(EXT_STR_h101, mosaic));
        source->AddReader(mosaic);
    }

    if (fMwpc0)
    {
        auto* mwpcreader = new R3BMwpcReader((EXT_STR_h101_SOFMWPC*)&ucesb_struct.mwpc, offsetof(EXT_STR_h101, mwpc));
        mwpcreader->SetMaxNbDet(1);
        source->AddReader(mwpcreader);
    }

    if (fFib30)
    {
        source->AddReader(new R3BFiberReader(
            "Fi30", 512, (EXT_STR_h101_FIBEO_onion*)&ucesb_struct.fib30, offsetof(EXT_STR_h101_t, fib30)));
    }

    if (fFib31)
    {
        source->AddReader(new R3BFiberReader(
            "Fi31", 512, (EXT_STR_h101_FIBEI_onion*)&ucesb_struct.fib31, offsetof(EXT_STR_h101_t, fib31)));
    }

    if (fFib32)
    {
        source->AddReader(new R3BFiberReader(
            "Fi32", 512, (EXT_STR_h101_FIBEZ_onion*)&ucesb_struct.fib32, offsetof(EXT_STR_h101_t, fib32)));
    }

    if (fFib33)
    {
        source->AddReader(new R3BFiberReader(
            "Fi33", 512, (EXT_STR_h101_FIBEE_onion*)&ucesb_struct.fib33, offsetof(EXT_STR_h101_t, fib33)));
    }

    if (fTofd)
    {
        source->AddReader(
            new R3BTofdReader((EXT_STR_h101_TOFD_onion*)&ucesb_struct.tofd, offsetof(EXT_STR_h101, tofd)));
    }

    if (fRpc)
    {
        source->AddReader(new R3BRpcReader((EXT_STR_h101_RPC*)&ucesb_struct.rpc, offsetof(EXT_STR_h101, rpc)));
    }

    if (fNeuland)
    {
        source->AddReader(new R3BNeulandTamexReader((EXT_STR_h101_raw_nnp_tamex_onion*)&ucesb_struct.neuland,
                                                    offsetof(EXT_STR_h101, neuland)));
        source->AddReader(new R3BWhiterabbitNeulandReader(
            (EXT_STR_h101_WRNEULAND*)&ucesb_struct.wrneuland, offsetof(EXT_STR_h101, wrneuland), 0x900));
    }

    run->SetSource(source);

    // Initialize  ----------------------------------------------------------
    timer.Start();
    run->Init();

    std::cout << "\n\n" << std::endl;
    std::cout << "Input file: " << filename << std::endl;
    std::cout << "\n\n" << std::endl;

    // Run -----------------------------------------------------------------
    run->Run((nev < 0) ? nev : 0, (nev < 0) ? 0 : nev);

    // Finish  -------------------------------------------------------------
    timer.Stop();
    Double_t rtime = timer.RealTime() / 60.;
    Double_t ctime = timer.CpuTime() / 60.;
    std::cout << std::endl << std::endl;
    std::cout << "Macro finished succesfully." << std::endl;
    std::cout << "Output file is " << outputFilename << std::endl;
    std::cout << "Real time " << rtime << " min, CPU time " << ctime << " min" << std::endl << std::endl;
    std::cout << "Macro finished successfully." << std::endl;
}
