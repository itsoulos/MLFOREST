# =============================================================================
#  ΝΕΟ ΣΕΝΑΡΙΟ: Πρόβλεψη κλίμακας πυρκαγιάς με FEATURES που ήδη γνωρίζουμε εκ των προτέρων
#  Από raw Excel Πυροσβεστικής → ARFF αρχεία για WEKA
# =============================================================================
#
#  ΕΚΤΕΛΕΣΗ ΤΟΥ:
#    python SENARIO_1_fire_pipeline.py
#
#  ΠΑΡΑΓΕΤΑΙ:
#    Φάκελος ARFF_NEW/ με τα νέα αρχεία ανά χρονιά
# =============================================================================

import pandas as pd
import numpy as np
import os
from sklearn.model_selection import StratifiedKFold

# =============================================================================
#  ΡΥΘΜΙΣΕΙΣ
# =============================================================================

# Τα Excel αρχεία της Πυροσβεστικής
ΑΡΧΕΙΑ = {
    2014: "Astika_Symvanta_2014.xlsx",
    2015: "Astika_Symvanta_2015_v1.1.xlsx",
    2016: "Astika_Symvanta_2016_v1.2.xlsx",
    2017: "Astika_Symvanta_2017_v1.3.xlsx",
    2018: "Astika_Symvanta_2018_v1.4.xlsx",
    2019: "Astika_Symvanta_2019_v1.5.xlsx",
    2020: "Astika_Symvanta_2020_v1.6.xlsx",
    2021: "Astika_Symvanta_2021.xlsx",
    2022: "Astika_Symvanta_2022_v1.7.xlsx",
    2023: "Astika_Symvanta_2023_v1.8.xlsx",
    2024: "Astika_symvanta_2024.xlsx",
    2025: "astika_symvanta_2025.xlsx",
}

# Φάκελος εξόδου
ΕΞΟΔΟΣ = "ARFF_NEW"

# Αντιστοίχιση class → αριθμός (ίδιο με paper για σύγκριση)
CLASS_MAP = {
    "ΜΙΚΡΗ":  11,
    "ΜΕΣΑΙΑ":  9,
    "ΜΕΓΑΛΗ":  8,
}

# =============================================================================
#  ΒΗΜΑ 1: Φόρτωση και καθαρισμός ενός Excel αρχείου
# =============================================================================

def fortose_excel(etos, path):
    """Φορτώνει το Excel, διαχειρίζεται διαφορές στη δομή ανά χρονιά."""
    
    # Το 2025 έχει header στη γραμμή 2 (0-indexed)
    header_row = 2 if etos == 2025 else 0
    
    df = pd.read_excel(path, header=header_row)
    print(f"  Φορτώθηκε: {len(df)} εγγραφές")
    
    # Ομογενοποίηση ονομάτων στηλών για το 2025
    # (έχει "ΑΣΤΙΚΗ ΠΥΡΚΑΓΙΑ" αντί "ΔΕΛΤΙΟ ΠΥΡΚΑΓΙΑΣ")
    if etos == 2025:
        df["Είδος Συμβάντος"] = df["Είδος Συμβάντος"].str.replace(
            "ΑΣΤΙΚΗ ΠΥΡΚΑΓΙΑ", "ΔΕΛΤΙΟ ΠΥΡΚΑΓΙΑΣ", regex=False
        ).str.replace(
            "ΠΑΡΟΧΗ ΒΟΗΘΕΙΑΣ", "ΔΕΛΤΙΟ ΠΑΡΟΧΗΣ ΒΟΗΘΕΙΑΣ", regex=False
        )
    
    return df


# =============================================================================
#  ΒΗΜΑ 2: Φιλτράρισμα — μόνο πυρκαγιές Μικρή/Μεσαία/Μεγάλη
# =============================================================================

def filtare(df):
    """Κρατάει μόνο πυρκαγιές με γνωστή κλίμακα."""
    
    # Μόνο αστικές πυρκαγιές
    df = df[df["Είδος Συμβάντος"].str.contains("ΠΥΡΚΑΓΙΑ", na=False)].copy()
    
    # Μόνο ΜΙΚΡΗ / ΜΕΣΑΙΑ / ΜΕΓΑΛΗ (όχι ψευδείς αναγγελίες κτλ.)
    df = df[df["Χαρακτηρισμός Συμβάντος"].isin(CLASS_MAP.keys())].copy()
    
    print(f"  Μετά φίλτρο: {len(df)} πυρκαγιές "
          f"(Μικρή:{(df['Χαρακτηρισμός Συμβάντος']=='ΜΙΚΡΗ').sum()} "
          f"Μεσαία:{(df['Χαρακτηρισμός Συμβάντος']=='ΜΕΣΑΙΑ').sum()} "
          f"Μεγάλη:{(df['Χαρακτηρισμός Συμβάντος']=='ΜΕΓΑΛΗ').sum()})")
    
    return df


# =============================================================================
#  ΒΗΜΑ 3: Εξαγωγή νέων features
# =============================================================================

def ekspase_features(df):
    """
    Εξάγει features που ΓΝΩΡΙΖΟΥΜΕ ΤΗ ΣΤΙΓΜΗ ΤΗΣ ΚΛΗΣΗΣ.
    Κανένα feature δεν απαιτεί γνώση του αποτελέσματος.
    """
    
    features = pd.DataFrame()
    
    # --- Χρονικά features (από ημερομηνία και ώρα) ---
    
    ημερομηνια = pd.to_datetime(
        df["Ημερ. Έναρξης Συμβάντος"], errors="coerce"
    )
    
    # Προσπαθεί να πάρει ώρα από τη στήλη Ώρα Έναρξης
    ωρα_str = df["Ώρα Έναρξης"].astype(str).str.extract(r"(\d{1,2}:\d{2})")[0]
    ωρα = pd.to_datetime(ωρα_str, format="%H:%M", errors="coerce").dt.hour
    
    # f1: Μήνας (1-12)
    features["f1_minas"] = ημερομηνια.dt.month
    
    # f2: Εποχή (1=Χειμώνας, 2=Άνοιξη, 3=Καλοκαίρι, 4=Φθινόπωρο)
    features["f2_epochi"] = ημερομηνια.dt.month.map({
        12: 1, 1: 1, 2: 1,   # Χειμώνας
         3: 2, 4: 2, 5: 2,   # Άνοιξη
         6: 3, 7: 3, 8: 3,   # Καλοκαίρι
         9: 4, 10: 4, 11: 4  # Φθινόπωρο
    })
    
    # f3: Ώρα ημέρας (0-23)
    features["f3_ora"] = ωρα
    
    # f4: Μέρα ή Νύχτα (1=Μέρα 7:00-20:59, 0=Νύχτα)
    features["f4_mera_nyxta"] = (
        ωρα.between(7, 20).astype(int)
    )
    
    # f5: Ημέρα εβδομάδας (0=Δευτέρα ... 6=Κυριακή)
    features["f5_imera_evdom"] = ημερομηνια.dt.dayofweek
    
    # f6: Σαββατοκύριακο (1=Σαβ/Κυρ, 0=Καθημερινή)
    features["f6_savvatokyriako"] = (
        ημερομηνια.dt.dayofweek >= 5
    ).astype(int)
    
    # f7: Εποχή (Τέταρτο) χρόνου (1=Ιαν-Μαρ, 2=Απρ-Ιουν, 3=Ιουλ-Σεπ, 4=Οκτ-Δεκ)
    features["f7_trimino"] = ημερομηνια.dt.quarter
    
    # --- Γεωγραφικά features ---
    
    # f8: Κωδικός Νομού (αριθμητικός)
    nomoi = df["Νομός"].fillna("ΑΓΝΩΣΤΟΣ").str.strip()
    nomas_codes = {v: i+1 for i, v in enumerate(sorted(nomoi.unique()))}
    features["f8_nomos"] = nomoi.map(namas_codes := nomas_codes)
    
    # f9: Κωδικός τύπου χώρου (Περιγραφή Χώρου)
    xoros = df["Περιγραφή Χώρου"].fillna("ΑΓΝΩΣΤΟΣ").str.strip()
    xoros_codes = {v: i+1 for i, v in enumerate(sorted(xoros.unique()))}
    features["f9_typos_xorou"] = xoros.map(xoros_codes)
    
    # --- Class ---
    features["class"] = df["Χαρακτηρισμός Συμβάντος"].map(CLASS_MAP)
    
    # Καθαρισμός NaN
    features = features.dropna()
    features = features.astype(int)
    
    print(f"  Features εξήχθησαν: {len(features)} καθαρές εγγραφές, "
          f"{len(features.columns)-1} features")
    
    return features, namas_codes, xoros_codes


# =============================================================================
#  ΒΗΜΑ 4: Δημιουργία ARFF αρχείου
# =============================================================================

def gramme_arff(df_features, path, relation_name, nomos_codes, xoros_codes):
    """Γράφει ένα ARFF αρχείο."""
    
    with open(path, "w", encoding="utf-8") as f:
        f.write(f"@Relation {relation_name}\n\n")
        
        # Ορισμός attributes
        f.write("@ATTRIBUTE f1_minas          NUMERIC  % Μήνας (1-12)\n")
        f.write("@ATTRIBUTE f2_epochi         NUMERIC  % Εποχή (1=Χειμ 2=Άνοιξ 3=Καλοκ 4=Φθιν)\n")
        f.write("@ATTRIBUTE f3_ora            NUMERIC  % Ώρα ημέρας (0-23)\n")
        f.write("@ATTRIBUTE f4_mera_nyxta     NUMERIC  % Μέρα=1 Νύχτα=0\n")
        f.write("@ATTRIBUTE f5_imera_evdom    NUMERIC  % Ημέρα εβδομάδας (0=Δευτ 6=Κυρ)\n")
        f.write("@ATTRIBUTE f6_savvatokyriako NUMERIC  % Σαββ/κο=1 Καθημ=0\n")
        f.write("@ATTRIBUTE f7_trimino        NUMERIC  % Τρίμηνο (1-4)\n")
        f.write("@ATTRIBUTE f8_nomos          NUMERIC  % Κωδικός Νομού\n")
        f.write("@ATTRIBUTE f9_typos_xorou    NUMERIC  % Κωδικός τύπου χώρου\n")
        f.write("@ATTRIBUTE class             {8,9,11} % 8=Μεγάλη 9=Μεσαία 11=Μικρή\n")
        f.write("\n@DATA\n")
        
        # Εγγραφές
        for _, row in df_features.iterrows():
            vals = [str(int(v)) for v in row]
            f.write(",".join(vals) + "\n")


# =============================================================================
#  ΒΗΜΑ 5: 10-fold split και αποθήκευση
# =============================================================================

def gramme_10fold(df_features, fakelos_etous, etos, 
                  nomos_codes, xoros_codes):
    """Κάνει 10-fold split και γράφει train/test ARFF για κάθε fold."""
    
    os.makedirs(fakelos_etous, exist_ok=True)
    
    X = df_features.drop("class", axis=1).values
    y = df_features["class"].values
    
    skf = StratifiedKFold(n_splits=10, shuffle=True, random_state=42)
    
    for fold, (train_idx, test_idx) in enumerate(skf.split(X, y), start=1):
        
        df_train = df_features.iloc[train_idx]
        df_test  = df_features.iloc[test_idx]
        
        train_path = os.path.join(fakelos_etous, 
                                  f"urban_new_{etos}_{fold}.train.arff")
        test_path  = os.path.join(fakelos_etous, 
                                  f"urban_new_{etos}_{fold}.test.arff")
        
        gramme_arff(df_train, train_path, 
                    f"urban_new_{etos}_fold{fold}_train",
                    nomos_codes, xoros_codes)
        gramme_arff(df_test, test_path,   
                    f"urban_new_{etos}_fold{fold}_test",
                    nomos_codes, xoros_codes)
    
    print(f"  ✅ 10 folds γραμμένα στο: {fakelos_etous}/")


# =============================================================================
#  ΚΥΡΙΑ ΕΚΤΕΛΕΣΗ
# =============================================================================

if __name__ == "__main__":
    
    print("=" * 65)
    print("  ΔΗΜΙΟΥΡΓΙΑ ΝΕΩΝ ARFF ΑΠΟ RAW DATA ΠΥΡΟΣΒΕΣΤΙΚΗΣ")
    print("  Features: Χρόνος κλήσης + Γεωγραφία (χωρίς αποτελέσματα)")
    print("=" * 65)
    
    os.makedirs(ΕΞΟΔΟΣ, exist_ok=True)
    
    # Αποθήκευση κωδικών για αναφορά
    ολοι_νομοι  = {}
    ολοι_χωροι  = {}
    συνοψη      = {}
    
    for etos, arxeio in ΑΡΧΕΙΑ.items():
        
        # Έλεγχος αν υπάρχει το αρχείο
        if not os.path.exists(arxeio):
            print(f"\n⚠ {etos}: Δεν βρέθηκε '{arxeio}' — παραλείπεται")
            continue
        
        print(f"\n{'─'*50}")
        print(f"  ΕΤΟΣ {etos}")
        print(f"{'─'*50}")
        
        # Βήμα 1: Φόρτωση
        df = fortose_excel(etos, arxeio)
        
        # Βήμα 2: Φιλτράρισμα
        df = filtare(df)
        
        if len(df) < 100:
            print(f"  ⚠ Πολύ λίγες εγγραφές ({len(df)}) — παραλείπεται")
            continue
        
        # Βήμα 3: Εξαγωγή features
        df_features, nomos_codes, xoros_codes = ekspase_features(df)
        
        # Αποθήκευση κωδικών
        ολοι_νομοι.update(nomos_codes)
        ολοι_χωροι.update(xoros_codes)
        συνοψη[etos] = len(df_features)
        
        # Βήμα 4+5: ARFF + 10-fold
        fakelos = os.path.join(ΕΞΟΔΟΣ, str(etos))
        gramme_10fold(df_features, fakelos, etos, nomos_codes, xoros_codes)
    
    # --- Αποθήκευση κωδικών για αναφορά ---
    import json
    with open(os.path.join(ΕΞΟΔΟΣ, "nomos_codes.json"), "w", 
              encoding="utf-8") as f:
        json.dump(ολοι_νομοι, f, ensure_ascii=False, indent=2)
    with open(os.path.join(ΕΞΟΔΟΣ, "xoros_codes.json"), "w", 
              encoding="utf-8") as f:
        json.dump(ολοι_χωροι, f, ensure_ascii=False, indent=2)
    
    # --- Σύνοψη ---
    print(f"\n{'='*65}")
    print("  ΣΥΝΟΨΗ:")
    print(f"{'='*65}")
    for etos, n in συνοψη.items():
        print(f"  {etos}: {n:>6} εγγραφές → 10 folds × 2 αρχεία = 20 ARFF")
    print(f"\n  Αποθηκεύτηκαν στο: {ΕΞΟΔΟΣ}/")
    print(f"  Κωδικοί νομών:  {ΕΞΟΔΟΣ}/nomos_codes.json")
    print(f"  Κωδικοί χώρων: {ΕΞΟΔΟΣ}/xoros_codes.json")
    print("\n  Έτοιμο για WEKA!")
