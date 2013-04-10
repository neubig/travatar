#ifndef _TERCPPTERCALC_H___
#define _TERCPPTERCALC_H___

#include <vector>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include "hashMap.h"
#include "hashMapInfos.h"
#include "hashMapStringInfos.h"
#include "terAlignment.h"
#include "tools.h"
#include "terShift.h"
#include "alignmentStruct.h"
#include "bestShiftStruct.h"

using namespace std;
using namespace Tools;
using namespace HashMapSpace;
namespace TERCpp
{
// typedef size_t WERelement[2];
// Vecteur d'alignement contenant le hash du mot et son evaluation (0=ok, 1=sub, 2=ins, 3=del)
    typedef vector<terShift> vecTerShift;
    /**
    	@author
    */
    class terCalc
    {
        private :
// Vecteur d'alignement contenant le hash du mot et son evaluation (0=ok, 1=sub, 2=ins, 3=del)
            WERalignment l_WERalignment;
// HashMap contenant les valeurs de hash de chaque mot
            hashMap bagOfWords;
            int TAILLE_PERMUT_MAX;
            // Increments internes  
            int NBR_SEGS_EVALUATED;
            int NBR_PERMUTS_CONSID;
            int NBR_BS_APPELS;
            int DIST_MAX_PERMUT;
            bool PRINT_DEBUG;

            // Utilisés dans minDistEdit et ils ne sont pas réajustés 
            double S[1000][1000];
            char P[1000][1000];
            vector<vecInt> refSpans;
            vector<vecInt> hypSpans;
            int TAILLE_BEAM;

        public:
            int shift_cost;
            int insert_cost;
            int delete_cost;
            int substitute_cost;
            int match_cost;
            double infinite;
            terCalc();

//     ~terCalc();
//             size_t* hashVec ( vector<string> s );
            void setDebugMode ( bool b );
//             int WERCalculation ( size_t * ref, size_t * hyp );
//             int WERCalculation ( vector<string> ref, vector<string> hyp );
//             int WERCalculation ( vector<int> ref, vector<int> hyp );
	    terAlignment WERCalculation ( vector<string> ref, vector<string> hyp );
// 	string vectorToString(vector<string> vec);
// 	vector<string> subVector(vector<string> vec, int start, int end);
            hashMapInfos createConcordMots ( vector<string> hyp, vector<string> ref );
            terAlignment minimizeDistanceEdition ( vector<string> hyp, vector<string> ref, vector<vecInt> curHypSpans );
            bool trouverIntersection ( vecInt refSpan, vecInt hypSpan );
            terAlignment TER ( vector<string> hyp, vector<string> ref , float avRefLength );
            terAlignment TER ( vector<string> hyp, vector<string> ref );
            terAlignment TER ( vector<int> hyp, vector<int> ref );
            bestShiftStruct findBestShift ( vector<string> cur, vector<string> hyp, vector<string> ref, hashMapInfos rloc, terAlignment cur_align );
            void calculateTerAlignment ( terAlignment align, bool* herr, bool* rerr, int* ralign );
            vector<vecTerShift> calculerPermutations ( vector<string> hyp, vector<string> ref, hashMapInfos rloc, terAlignment align, bool* herr, bool* rerr, int* ralign );
            alignmentStruct permuter ( vector<string> words, terShift s );
            alignmentStruct permuter ( vector<string> words, int start, int end, int newloc );
    };

}

#endif
