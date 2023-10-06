#include "create_new_contigs.h"

#include <algorithm> //for "sort"
#include <fstream>
#include <omp.h>
#include <tuple>
#include "input_output.h"
#include "tools.h"
// #include "reassemble_unaligned_reads.h"
#include "edlib/include/edlib.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::list;
using std::vector;
using std::min;
using std::max;
using std::begin;
using std::end;
using std::unordered_map;
using std::pair;
using std::make_pair;
using std::set;
using std::to_string;
using std::stoi;
using std::stof;
using std::get; //in tuple
using std::ofstream;

/**
 * @brief Parse the file containing the partitions of the reads
 * 
 * @param file 
 * @param allreads 
 * @param allOverlaps Will be filled by this function
 * @param partitions Structure that will contain the partitions
 */
void parse_split_file(
    std::string& file, 
    std::vector <Read> &allreads,
    std::vector <Overlap> &allOverlaps,
    std::unordered_map<unsigned long int ,std::vector< std::pair<std::pair<int,int>, std::vector<int> > > > &partitions){

    robin_hood::unordered_map<std::string, int> name_of_contigs;
    for (int i = 0 ; i < allreads.size() ; i++){
        name_of_contigs[allreads[i].name] = i;
    }

    //open the file
    std::ifstream infile(file);
    if (!infile.good()){
        cerr << "ERROR: could not open file " << file << endl;
        exit(1);
    }

    //read the file
    string line;
    long int contig;
    
    std::unordered_map<string, int> name_of_neighbors;
    vector<string> list_of_read_names;
    while (std::getline(infile, line)){
        //read the first field of the line
        std::istringstream iss(line);
        string category;
        iss >> category;
        if (category == "CONTIG"){
            //reset the variables
            name_of_neighbors.clear();
            list_of_read_names.clear();

            //parse the name of the contig
            string contigName;
            iss >> contigName;

            contig = name_of_contigs[contigName];
            int length;
            double depth;
            iss >> length >> depth;
            allreads[contig].depth = depth;
            partitions[contig] = {};

            //go through the neigbors and inventoriate their names
            for(int n = 0 ; n < allreads[contig].neighbors_.size() ; n++){
                int overlap = allreads[contig].neighbors_[n];
                string neighborName = allreads[allOverlaps[overlap].sequence1].name;
                name_of_neighbors[neighborName] = n;
            }
        }
        else if (category == "READ"){
            //parse the name of the read
            string readName;
            int startRead, endRead, startContig, endContig;
            bool strand;
            iss >> readName >> startRead >> endRead >> startContig >> endContig >> strand;
            list_of_read_names.push_back(readName);
            long int read = name_of_contigs[readName];
            //add the overlap between the read and the contig (if strand != -1)
            // if (strand != -1){
            //     int overlap = allOverlaps.size();
            //     Overlap o;
            //     o.sequence1 = read;
            //     o.sequence2 = contig;
            //     o.position_1_1 = startRead;
            //     o.position_1_2 = endRead;
            //     o.position_2_1 = startContig;
            //     o.position_2_2 = endContig;
            //     o.strand = strand;

            //     allOverlaps.push_back(o);

            //     allreads[read].add_overlap(overlap);
            //     allreads[contig].add_overlap(overlap);
            // }
        }
        else if (category == "GROUP"){
            //parse the coordinates of the group
            int start, end;
            iss >> start >> end;

            //parse the index of reads present there
            std::vector<int> readIdxs;
            string readIdxsString;
            string partitionString;
            iss >> readIdxsString >> partitionString;
            // readIdxsString is a comma-separated list of integers
            std::istringstream iss2(readIdxsString);
            string number;

            // cout << "readIdxsString " << readIdxsString << endl;
            // cout << "partitionString " << partitionString << endl;
            
            if (readIdxsString != "," && partitionString != ","){

                while (std::getline(iss2, number, ',')){
                    readIdxs.push_back(std::stoi(number));
                }
                
                //now parse the partition
                std::vector<int> partition;

                // partitionString is a comma-separated list of integers
                std::istringstream iss3(partitionString);
                while (std::getline(iss3, number, ',')){
                    partition.push_back(std::stoi(number));
                }

                vector<int> partitions_with_the_minus_2 (allreads[contig].neighbors_.size(), -2); //to spare memory, -2 were not stored in the file, so we need to add them back
                
                for (int r = 0 ; r < readIdxs.size() ; r++){
                    // cout << "list_of_reads_names " << endl;
                    // for (auto i : list_of_read_names){
                    //     cout << i << " ";
                    // }
                    if (name_of_neighbors.find(list_of_read_names[readIdxs[r]]) != name_of_neighbors.end()){ //this can happen for overlaps that are discarded in read_SAM but were not in the gro file
                        partitions_with_the_minus_2[ name_of_neighbors[list_of_read_names[readIdxs[r]]]] = partition[r];
                    }
                }
                // int index = 0;
                // for (int r =  0 ; r < readIdxs.size() ; r++){
                //     while (index < readIdxs[r]){
                //         partitions_with_the_minus_2.push_back(-2);
                //         index++;
                //     }
                //     partitions_with_the_minus_2.push_back(partition[r]);
                //     index++;
                // }
                // while (index < allreads[contig].neighbors_.size()){
                //     partitions_with_the_minus_2.push_back(-2);
                //     index++;
                // }
                //now append to partitions[contig]
                partitions[contig].push_back(std::make_pair(std::make_pair(start, end), partitions_with_the_minus_2));
            }
        }
    }
}

/**
 * @brief Modify the input GFA according to the way the reads have been split.
 * 
 * @param readsFile File containing all the reads
 * @param allreads vector containing all the reads (without their actual sequence)
 * @param backbones_reads vector listing all the backbone reads
 * @param allOverlaps vector containing all the overlaps
 * @param partitions for each backbone, contains a vector of the intervals, and for each interval, the partition of the reads
 * @param allLinks vector containing all the links of the GFA file (new one will be added)
 * @param readLimits for each backbone, contains the limits (in term of coordinates) of all its neighbors on the backbone
 * @param num_threads number of threads to use
 * @param techno technology used to generate the reads (ont, pacbio, hifi)
 */
void modify_GFA(
    std::string readsFile, 
    vector <Read> &allreads, 
    vector<unsigned long int> &backbones_reads, 
    vector <Overlap> &allOverlaps,
    std::unordered_map<unsigned long int ,std::vector< std::pair<std::pair<int,int>, std::vector<int> > > > &partitions,
    vector<Link> &allLinks,
    int num_threads,
    string &outFolder, 
    float errorRate,
    std::string &polisher,
    bool polish,
    string &techno,
    string &MINIMAP, 
    string &RACON,
    string &MEDAKA,
    string &SAMTOOLS,
    string &path_to_python,
    string &path_src,
    bool DEBUG)
    {

    int max_backbone = backbones_reads.size(); //fix that because backbones will be added to the list but not separated 
    string log_text = ""; //text that will be printed out in the output.txt

    omp_set_num_threads(num_threads);
    #pragma omp parallel for
    for (int b = 0 ; b < max_backbone ; b++){

        //first load all the reads
        #pragma omp critical
        {
            parse_reads_on_contig(readsFile, backbones_reads[b], allOverlaps, allreads);
        }

        // if (allreads[backbones_reads[b]].name != "edge_10"){
        //     cout << "edgge 10" << endl;
        //     continue;
        // }

        //then separate the contigs

        string local_log_text = ""; //text that will be printed out in the output.txt generated in this thread
        if (DEBUG){
            #pragma omp critical
            {
                cout << "Thread " << omp_get_thread_num() << " looking at " << allreads[backbones_reads[b]].name  << endl;
            }
        }
        // if (allreads[backbones_reads[b]].name != "contig_339@0"){ //DEBUG
        //     // cout << "continuuinng" << endl;
        //     continue;
        // }

        local_log_text += "---- contig: " + allreads[backbones_reads[b]].name + " ----\n\n";

        string thread_id = std::to_string(omp_get_thread_num());
        int backbone = backbones_reads[b];

        //if partitions[backbone].size() == 0, see if we need to repolish or not, depending on wether the coverage is coherent or not
        bool dont_recompute_contig = false;
        if (partitions[backbone].size() == 0 && allreads[backbones_reads[b]].depth > 1){
            double total_depth = 0;
            for (auto n : allreads[backbones_reads[b]].neighbors_){
                total_depth += allOverlaps[n].position_1_2 - allOverlaps[n].position_1_1;
            }
            double new_depth = total_depth / allreads[backbones_reads[b]].sequence_.size();

            if (new_depth / allreads[backbones_reads[b]].depth > 0.7){
                dont_recompute_contig = true;
            }
        }

        if (partitions.find(backbone) != partitions.end() && partitions[backbone].size() > 0 && !dont_recompute_contig){
            //stitch all intervals of each backbone read
            vector<unordered_map <int,set<int>>> stitches(partitions[backbone].size()); //aggregates the information from stitchesLeft and stitchesRight to know what link to keep

            // //if readLimits is not computed, provide an estimation
            // if (readLimits[backbone].size() == 0){
            //     for (int neighbor = 0 ; neighbor < allreads[backbone].neighbors_.size() ; neighbor++){
            //         int overlap = allreads[backbone].neighbors_[neighbor];
            //         readLimits[backbone].push_back(std::make_pair(allOverlaps[overlap].position_2_1, allOverlaps[overlap].position_2_2));
            //     }
            // }

            for (int n = 0 ; n < partitions[backbone].size() ; n++){
                //for each interval, go through the different parts and see with what part before and after they fit best
                if (n > 0){
                    std::unordered_map<int, std::set<int>> stitchLeft = stitch(partitions[backbone][n].second, 
                                                                            partitions[backbone][n-1].second, 
                                                                            partitions[backbone][n].first.first);
                    // std::unordered_map<int, std::set<int>> stitchRight = stitch(
                    //                                                         partitions[backbone][n-1].second.first,
                    //                                                         partitions[backbone][n].second.first,
                    //                                                         partitions[backbone][n].first.first, 
                    //                                                         readLimits[backbone]);


                    for (auto s : stitchLeft){
                        stitches[n][s.first] = s.second;
                    }

                    // for (auto s : stitchRight){
                    //     for (int neighbor : s.second){
                    //         stitches[n][neighbor].emplace(s.first);
                    //     }
                    // }

                    //make sure all contigs on the left and right are stitched
                    set<int> all_contigs_left;
                    for (int a : partitions[backbone][n-1].second){
                        all_contigs_left.emplace(a);
                    }
                    all_contigs_left.erase(-1);
                    all_contigs_left.erase(-2);

                    set<int> all_contigs_right;
                    for (int a : partitions[backbone][n].second){
                        all_contigs_right.emplace(a);
                    }
                    all_contigs_right.erase(-1);
                    all_contigs_right.erase(-2);

                    for (auto s : stitchLeft){
                        if (s.second.size() == 0){
                            stitchLeft[s.first] = all_contigs_left;
                        }
                    }

                    //check if all contigs on the left of the junction are stitched
                    set <int> stitchedContigs;
                    for (auto s : stitches[n]){
                        for (int neighbor : s.second){
                            stitchedContigs.emplace(neighbor);
                        }
                    }
                    for (auto contig : all_contigs_left){
                        if (stitchedContigs.find(contig) == stitchedContigs.end()){
                            for (auto s : stitchLeft){
                                stitches[n][s.first].emplace(contig);
                            }
                        }
                    }
                    // cout << "stitched contigs : "; for(auto i : stitchedContigs) {cout <<i << ";";} cout << endl;
                    // for (auto s : stitchRight){
                    //     if (stitchedContigs.find(s.first) == stitchedContigs.end()){
                    //         // cout << "gluing back" << endl;
                    //         for (auto s2 : stitchLeft){
                    //             stitches[n][s2.first].emplace(s.first);
                    //         }
                    //     }
                    // }

                    // if (partitions[backbone][n].first.first > 63100 && partitions[backbone][n].first.first < 64900 ){
                    //     cout << "stiddvtching : ";
                    //     for (auto s: stitches[n]){
                    //         cout << s.first << "<->" ;
                    //         for (auto s2 : s.second) {cout << s2 << ",";}
                    //         cout << "  ;  ";
                    //     } 
                    //     cout << endl;
                    // }
                        
                    // }
                }
            }


            //create hangingLinks, a list of links that are not yet connected but will soon be
            vector<int> hangingLinks;
            for (int linkIdx : allreads[backbone].get_links_left()){
                if (allLinks[linkIdx].neighbor1 == backbone && allLinks[linkIdx].end1 == 0){
                    allLinks[linkIdx].end1 = -1;
                }
                else {
                    allLinks[linkIdx].end2 = -1;
                }
                allLinks[linkIdx].group = 0;
                hangingLinks.push_back(linkIdx);
            }

            //compute the depth from the number of aligning reads
            std::pair<int,int> limitsAll= std::make_pair(0, allreads[backbone].sequence_.size()-1);
            vector<int> partition1 (allreads[backbone].neighbors_.size(), 1);
            unordered_map <int, double> newdepths = recompute_depths(limitsAll , partition1, allreads[backbone].depth);

            int n = 0;
            for (auto interval : partitions[backbone]){

                // if (interval.first.first == 36000){
                //     cout  << "fdiocicui modufy_gfa" << endl;
                //     for (auto i = 0 ; i < interval.second.first.size() ; i++ ){
                //         if (interval.second.first[i] != -2){
                //             cout << interval.second.first[i]  << " ";
                //         }
                //     }
                //     cout << endl;
                // }

                unordered_map<int, vector<string>> readsPerPart; //list of all reads of each part
                unordered_map<int, vector<string>> fullReadsPerPart; //list of all reads of each part
                unordered_map<int, vector<pair<string, int>>> CIGARsPerPart; //list of all CIGARS of each part and the starting position
                int overhang = 150; //margin we're taking at the ends of the contig t get a good polishing of first and last bases
                int overhangLeft = min(interval.first.first, overhang);
                int overhangRight = max(0,min(int(allreads[backbone].sequence_.size())-interval.first.second-1, overhang));
                int leftToPolish = max(0, interval.first.first - overhangLeft);
                int rightToPolish = min(int(allreads[backbone].sequence_.size())-1, interval.first.second + overhangRight +1);
                if (omp_get_thread_num() == 0 && DEBUG){
                    cout << "in interval " << interval.first.first << " <-> " << interval.first.second << endl;
                }
                local_log_text += " - Between positions " + to_string(interval.first.first) + " and " + to_string(interval.first.second) + " of the contig, I've created these contigs:\n";

                int numberOfClusters = 0;
                set<int> existingparts;
                for (int r = 0 ; r < interval.second.size(); r++){
                    if (interval.second[r] > -1){

                        auto idxRead = allOverlaps[allreads[backbone].neighbors_[r]].sequence1;

                        int clust = interval.second[r];
                        existingparts.emplace(clust);
                        // string clippedRead = allreads[idxRead].sequence_.str();
                        //extract the part of the read that is on the interval and the part of the CIGAR that is on the interval
                        int posOnRead = 0;
                        int posOnCIGAR = 0;
                        int startPosition = allOverlaps[allreads[backbone].neighbors_[r]].position_2_1+1; //that's for the sam file
                        startPosition = max(1, startPosition - leftToPolish);
                        int posOnInterval = allOverlaps[allreads[backbone].neighbors_[r]].position_2_1;
                        // posOnInterval = 0;
                        int posOnReadStart = -1;
                        int posOnReadEnd = -1;
                        int posOnCIGARStart = -1;
                        int posOnCIGAREnd = -1;
                        string converted_cigar = convert_cigar(allOverlaps[allreads[backbone].neighbors_[r]].CIGAR);
                        for (char c : converted_cigar){

                            posOnCIGAR++;
                            if (c == 'S' || c == 'H'){ //get to the beginning of the read
                                posOnRead++;
                                continue;
                            }
            
                            if (posOnReadStart == -1 && posOnInterval >= leftToPolish){
                                // cout << "bingo posOnReadStart " << posOnRead << " " << posOnCIGAR << " " << posOnInterval << " " << leftToPolish << endl;
                                posOnReadStart = posOnRead;
                                posOnCIGARStart = posOnCIGAR-1;
                            }
                            if (posOnReadEnd == -1 && posOnInterval == rightToPolish){ 
                                posOnReadEnd = posOnRead;
                                posOnCIGAREnd = posOnCIGAR-1;
                                break;
                            }
      
                            if (c == 'M'){
                                posOnRead++;
                                posOnInterval++;
                            }
                            else if (c == 'D'){
                                posOnInterval++;
                            }
                            else if (c == 'I'){
                                posOnRead++;
                            }
                        }

                        if (posOnReadEnd == -1){
                            posOnReadEnd = posOnRead;
                            posOnCIGAREnd = posOnCIGAR;
                        }

                        if (posOnReadStart > posOnReadEnd || posOnReadStart == -1){ //can happen when within a deletion
                            interval.second[r] = -2;
                            continue;
                        }

                        // cout << "overylap : " << allreads[idxRead].name << " " << allOverlaps[allreads[backbone].neighbors_[r]].position_1_1 << " " << allOverlaps[allreads[backbone].neighbors_[r]].position_1_2 << " " << allOverlaps[allreads[backbone].neighbors_[r]].position_2_1 << " " << allOverlaps[allreads[backbone].neighbors_[r]].position_2_2 << endl;
                        // cout << "fsljd iid " << leftToPolish << " " << rightToPolish << endl;
                        // cout << "foiupo q " << posOnReadStart << " " << posOnReadEnd << " " << posOnCIGARStart << " " << posOnCIGAREnd << endl;
                
                        string seq = allreads[idxRead].sequence_.str();
                        if (!allOverlaps[allreads[backbone].neighbors_[r]].strand){
                            seq = allreads[idxRead].sequence_.reverse_complement().str();
                        }
                        string clippedRead = seq.substr(posOnReadStart, posOnReadEnd-posOnReadStart);
                        string clippedCIGAR = converted_cigar.substr(posOnCIGARStart, posOnCIGAREnd-posOnCIGARStart);

                        clippedCIGAR = convert_cigar2(clippedCIGAR); //convert back MMMDDMM -> 3M2D2M
                        // cout << "fjkld " << allOverlaps[allreads[backbone].neighbors_[r]].CIGAR << endl;

                        // int limitLeft = max(0,allOverlaps[allreads[backbone].neighbors_[r]].position_1_1-20); //the limit of the read that we should use with a little margin for a clean polish
                        // int limitRight = min(allOverlaps[allreads[backbone].neighbors_[r]].position_1_2+20, int(allreads[idxRead].sequence_.size()));

                        // string clippedRead; //the read we're aligning with good orientation and only the part we're interested in
                        
                        // cout << "cliipplling read: " << allreads[idxRead].name << " " << posOnReadStart << " " << posOnReadEnd << " " << 
                        //     allOverlaps[allreads[backbone].neighbors_[r]].position_1_1 << " " << allOverlaps[allreads[backbone].neighbors_[r]].position_1_2 << " " <<
                        //     allOverlaps[allreads[backbone].neighbors_[r]].position_2_1 << " " << allOverlaps[allreads[backbone].neighbors_[r]].position_2_2 << " " <<
                        //     clippedRead.substr(0,30) << endl;

                        if (readsPerPart.find(clust) == readsPerPart.end()){
                            readsPerPart[clust] = {clippedRead};
                            fullReadsPerPart[clust] = {allreads[idxRead].sequence_.str()};
                            CIGARsPerPart[clust] = {make_pair(clippedCIGAR, startPosition)};
                            if (clust >= 0){
                                numberOfClusters++;
                            }
                        }
                        else {
                            readsPerPart[clust].push_back(clippedRead);
                            fullReadsPerPart[clust].push_back(allreads[idxRead].sequence_.str());
                            CIGARsPerPart[clust].push_back(make_pair(clippedCIGAR, startPosition));
                        }
                    }
                }
                if (readsPerPart.size() == 0 && interval.second.size() > 0 && interval.second[0] <= -1){
                    if (existingparts.size() == 0){
                        readsPerPart[-1] = {}; //so that it defaults back to the consensus
                        fullReadsPerPart[-1] = {};
                        CIGARsPerPart[-1] = {};
                    }
                }
                for (int clust : existingparts){ //the partitions exist but with no reads
                    if (readsPerPart.find(clust) == readsPerPart.end()){
                        readsPerPart[clust] = {}; //so that it defaults back to the consensus
                        fullReadsPerPart[clust] = {};
                        CIGARsPerPart[clust] = {};
                    }
                }

                vector<int> futureHangingLinks;

                unordered_map <int, double> newdepths = recompute_depths(interval.first, interval.second, allreads[backbone].depth);

                for (auto group : readsPerPart){
                    
                    string full_backbone =  allreads[backbone].sequence_.str();
                    //toPolish should be polished with a little margin on both sides to get cleanly first and last base
                    string toPolish = full_backbone.substr(max(0, interval.first.first - overhangLeft), min(overhangLeft, interval.first.first)) 
                        + full_backbone.substr(interval.first.first, interval.first.second-interval.first.first)
                        + full_backbone.substr(interval.first.second, min(overhangRight+1, int(allreads[backbone].sequence_.size())-interval.first.second-1));


                    string newcontig = "";
                    if (numberOfClusters > 1 || polish){

                        // cout << "polishing with " << polisher << " iuce contig " << allreads[backbone].name + "_"+ to_string(interval.first.first)+ "_" + to_string(group.first) << " " << group.second.size() << endl;
                        if (group.second.size() == 0){
                            newcontig = "";
                        }
                        if (polisher == "medaka"){
                            newcontig = consensus_reads_medaka(toPolish, group.second, thread_id, outFolder, MEDAKA, SAMTOOLS, path_to_python, path_src);
                        }
                        else{
                            // newcontig = toPolish; //DEBUG
                            newcontig = consensus_reads(toPolish, full_backbone, 
                                interval.first.first, interval.first.second-interval.first.first+1, group.second, fullReadsPerPart[group.first], CIGARsPerPart[group.first], 
                                    thread_id, outFolder, techno, MINIMAP, RACON, path_to_python, path_src);
                            // if (interval.first.first == 38000 && group.first == 1){
                            //     cout << "fqljkd uciupiou edge_111@0_38000_1" << endl;
                            //     exit(1);
                            // }
                        }
                        // if (newcontig == ""){
                            // newcontig = consensus_reads(toPolish, full_backbone, 
                            //     interval.first.first, interval.first.second-interval.first.first+1, group.second, thread_id, outFolder, techno, MINIMAP, RACON);
                        // }
                        
                        // string samtools = "samtools";
                        // string wtdbg2 = "/home/rfaure/Documents/software/wtdbg2/wtdbg2";
                        // newcontig = consensus_reads_wtdbg2(toPolish, group.second, thread_id, outFolder, techno, MINIMAP, RACON, samtools, wtdbg2 );
                        if (newcontig != ""){

                            EdlibAlignResult result = edlibAlign(toPolish.c_str(), toPolish.size(),
                                        newcontig.c_str(), newcontig.size(),
                                        edlibNewAlignConfig(-1, EDLIB_MODE_HW, EDLIB_TASK_PATH, NULL, 0));

                            string cig = edlibAlignmentToCigar(result.alignment, result.alignmentLength, EDLIB_CIGAR_STANDARD);
                            string cigar = convert_cigar(cig);
                            // extract the part of newcontig that does not align to the first and last overhang bases of toPolish2
                            int posOnToPolish = 0;
                            int posOnNewContig = result.startLocations[0];
                            int posStartOnNewContig = 0;
                            int posEndOnNewContig = 0;
                            // cout << "fqjdk " << posOnNewContig << " " << overhangLeft << " " << overhangRight << " " << toPolish.size() << endl;
                            for (char c : cigar){
                                if (c == 'M'){
                                    posOnToPolish++;
                                    posOnNewContig++;
                                }
                                else if (c == 'D'){
                                    posOnNewContig++;
                                }
                                else if (c == 'I'){
                                    posOnToPolish++;
                                }
                                if (posOnToPolish == overhangLeft+1){
                                    posStartOnNewContig = posOnNewContig;
                                }
                                if (posOnToPolish == toPolish.size()-overhangRight){
                                    posEndOnNewContig = posOnNewContig;
                                }
                                // cout << "indices: " << posOnToPolish << " " << posOnNewContig << endl;
                            }
                            
                            newcontig = newcontig.substr(posStartOnNewContig, min(posEndOnNewContig-posStartOnNewContig+1, int(newcontig.size())-posStartOnNewContig));
                            // cout << "kmljlkmjlkjijkl " << newcontig.substr(0,50) << endl << endl;
                            edlibFreeAlignResult(result);
                        }
                    }
                    else {
                        if (group.second.size() == 0 && group.first != -1){ //if there is a cluster, but only with deletions
                            newcontig = "";
                        }
                        else{
                            newcontig = allreads[backbone].sequence_.str().substr(interval.first.first, interval.first.second-interval.first.first+1);
                        }
                    }

                    Read r(newcontig, newcontig.size());
                    r.name = allreads[backbone].name + "_"+ to_string(interval.first.first)+ "_" + to_string(group.first);
                    if (readsPerPart.size() > 1){
                        r.depth = newdepths[group.first];
                    }
                    else {
                        r.depth = allreads[backbone].depth;
                    }

                    // if (r.name == "s0.ctg000001l@1_84000_2"){
                    //     cout << "oiaooeiiddz" << endl;
                    //     exit(1);
                    // }

                    //now create all the links IF they are compatible with "stitches"  
                    set<int> linksToKeep;

        
                    if (n == 0 || stitches[n][group.first].size() == 0){
                        for (int h : hangingLinks){
                            linksToKeep.emplace(allLinks[h].group);
                        }
                    }
                    else{
                        for (int l : stitches[n][group.first]){
                            linksToKeep.emplace(l);
                        }
                    }
                    
                    //create the links
                    #pragma omp critical
                    {
                        for (int h : hangingLinks){
                            if (linksToKeep.find(allLinks[h].group) != linksToKeep.end()){
                                Link leftLink;
                                leftLink.CIGAR = allLinks[h].CIGAR;
                                if (allLinks[h].end2 == -1){
                                    leftLink.end2 = 0;
                                    leftLink.neighbor2 = allreads.size();
                                    leftLink.end1 = allLinks[h].end1;
                                    leftLink.neighbor1 = allLinks[h].neighbor1;
                                    allreads[leftLink.neighbor1].add_link(allLinks.size(), allLinks[h].end1);
                                }
                                else if (allLinks[h].end1 == -1) {
                                    leftLink.end1 = 0;
                                    leftLink.neighbor1 = allreads.size();
                                    leftLink.end2 = allLinks[h].end2;
                                    leftLink.neighbor2 = allLinks[h].neighbor2;
                                    allreads[leftLink.neighbor2].add_link(allLinks.size(), allLinks[h].end2);
                                }
                                else {
                                    // cout << "WHAAAT" << endl;
                                }
                                r.add_link(allLinks.size(), 0);
                                allLinks.push_back(leftLink);
                            }
                        }
                    
                        Link rightLink;
                        rightLink.CIGAR = "0M";
                        rightLink.end1 = 1;
                        rightLink.neighbor1 = allreads.size();
                        rightLink.end2 = -1;
                        rightLink.group = group.first;
                        allLinks.push_back(rightLink);
                        r.add_link(allLinks.size()-1, 1);
                        futureHangingLinks.push_back(allLinks.size()-1);

                        allreads.push_back(r);
                        backbones_reads.push_back(allreads.size()-1);
                        if (omp_get_thread_num() == 0 && DEBUG){
                            cout << "created the contig " << r.name << endl;
                        }
                        local_log_text += "   - " + r.name + "\n";
                    }
                }
                hangingLinks = futureHangingLinks;
                n += 1;
            }
            //now wrap up the right of the contig
            int left = partitions[backbone][partitions[backbone].size()-1].first.second+1; //rightmost interval
            string right;
            if (left < allreads[backbone].sequence_.size()){
                right = allreads[backbone].sequence_.str().substr(left, allreads[backbone].sequence_.size()-left);
            }
            else {
                right = "";
            }
            std::pair<int,int> limits= std::make_pair(left, allreads[backbone].sequence_.size()-1);
            string contig = right;
            
            Read r (contig, contig.size());
            r.name = allreads[backbone].name + "_"+ to_string(left)+ "_" + to_string(0);
            r.depth = newdepths[1];
            
            #pragma omp critical
            {
                for (int h : hangingLinks){
                    Link leftLink;
                    leftLink.CIGAR = allLinks[h].CIGAR;
                    if (allLinks[h].end2 == -1){
                        leftLink.end2 = 0;
                        leftLink.neighbor2 = allreads.size();
                        leftLink.end1 = allLinks[h].end1;
                        leftLink.neighbor1 = allLinks[h].neighbor1;
                        allreads[leftLink.neighbor1].add_link(allLinks.size(), allLinks[h].end1);
                    }
                    else if (allLinks[h].end1 == -1) {
                        leftLink.end1 = 0;
                        leftLink.neighbor1 = allreads.size();
                        leftLink.end2 = allLinks[h].end2;
                        leftLink.neighbor2 = allLinks[h].neighbor2;
                        allreads[leftLink.neighbor2].add_link(allLinks.size(), allLinks[h].end2);
                    }
                    else {
                        // cout << "WHAAAT" << endl;
                    }
                    r.add_link(allLinks.size(), 0);
                    allLinks.push_back(leftLink);
                    
                }

                //now re-build the links right of backbone
                for (int linkIdx : allreads[backbone].get_links_right()){
                    if (allLinks[linkIdx].neighbor1 == backbone && allLinks[linkIdx].end1 == 1){
                        allLinks[linkIdx].end1 = 1;
                        allLinks[linkIdx].neighbor1 = allreads.size() ;
                    }
                    else {
                        allLinks[linkIdx].end2 = 1;
                        allLinks[linkIdx].neighbor2 = allreads.size() ;
                    }
                    r.add_link(linkIdx, 1);
                }

                allreads.push_back(r);
                backbones_reads.push_back(allreads.size()-1);
                if (omp_get_thread_num() == 0 && DEBUG){
                    cout << "now creating the different contigs : " << r.name << endl;
                }
                local_log_text += " - Between positions " + to_string(left) + " and " + to_string(allreads[backbone].sequence_.size()) + " of the contig, I've created these contigs:\n";
                local_log_text +=  "   - " + r.name + "\n\n";

                allreads[backbone].name = "delete_me"; //output_gfa will understand that and delete the contig
            }

            // if (allreads[allreads.size()-1].name.substr(0,9) == "edge_13@0"){
            //     cout << "qfdklmdjlccjj " << endl;
            //     exit(1);
            // }

        }
        else{
            local_log_text += "Nothing to do\n\n";
        }
        #pragma omp critical
        {
            log_text += local_log_text;
        }

        //free up memory by deleting the sequence of the reads used there
        for (auto n : allreads[backbones_reads[b]].neighbors_){
            if (allOverlaps[n].sequence1 != backbones_reads[b]){
                allreads[allOverlaps[n].sequence1].free_sequence();
            }
            else{
                allreads[allOverlaps[n].sequence2].free_sequence();
            }
        }
    }

    std::ofstream o("output.txt");
    o << log_text << endl;
}

/**
 * @brief Tells to which parts of neighbor each part of par should be linked
 * 
 * @param par partitions on the partition
 * @param neighbor partitions on the neighbor
 * @param position position of the stitch in the backbone
 * @param readLimits limits of the reads in the backbone (so that reads that are not on the position of the stitch are not considered)
 * @return unordered_map<int, set<int>> map associating a set of partitions of neighbor matching each partition of par
 */
unordered_map<int, set<int>> stitch(vector<int> &par, vector<int> &neighbor, int position){

    unordered_map<int, unordered_map<int,int>> fit_left; //each parts maps to what left part ?
    unordered_map<int, unordered_map<int,int>> fit_right; //each parts maps to what right part ?
    unordered_map<int, int> cluster_size; 
    unordered_map<int,set<int>> stitch;

    for (auto r = 0 ; r < par.size() ; r++){
        // if (par[r] == 1 && position == 64000){
        //     cout << "parttiion " << par[r] << " neighbor " << neighbor[r] << endl;
        // }
        if (par[r] > -1 && neighbor[r] > -1){
            if (fit_left.find(par[r]) != fit_left.end()){
                if (fit_left[par[r]].find(neighbor[r]) != fit_left[par[r]].end()){
                    fit_left[par[r]][neighbor[r]] += 1;
                }
                else{
                    fit_left[par[r]][neighbor[r]] = 1;
                }
                cluster_size[par[r]] += 1;
            }
            else{
                fit_left[par[r]][neighbor[r]] = 1;
                cluster_size[par[r]] = 1;
                stitch[par[r]] = {};
            }

            if (fit_right.find(neighbor[r]) != fit_right.end()){
                if (fit_right[neighbor[r]].find(par[r]) != fit_right[neighbor[r]].end()){
                    fit_right[neighbor[r]][par[r]] += 1;
                }
                else{
                    fit_right[neighbor[r]][par[r]] = 1;
                }
            }
            else{
                fit_right[neighbor[r]][par[r]] = 1;
            }

        }
    }
    // if (position == 64000){
    //     cout << "here isss the fit_left" << endl;
    //     for (auto fit : fit_left){
    //         cout << fit.first << " : " << endl;
    //         for (auto candidate : fit.second){
    //             cout << "   " << candidate.first << " : " << candidate.second << endl;
    //         }
    //     }
    // }

    //now give all associations
    for (auto fit : fit_left){
        for (auto candidate : fit.second){
            if (candidate.second >= min(5.0, 0.7*cluster_size[fit.first])){ //good compatibility
                stitch[fit.first].emplace(candidate.first);
            }
        }
    }

    for (auto fit : fit_right){
        //find all the good fits
        for (auto candidate : fit.second){
            if (candidate.second >= min(5.0, 0.7*cluster_size[candidate.first])){
                stitch[candidate.first].emplace(fit.first);
            }
        }
    }

    return stitch;
}

//input : an interval, the list of the limits of the reads on the backbone, the depth of the contig of origin
//output : the recomputed read coverage for each of the new contigs, (scaled so that the total is the original depth)
std::unordered_map<int, double> recompute_depths(std::pair<int,int> &limits, std::vector<int> &partition, double originalDepth){

    unordered_map <int, double> newCoverage;
    int lengthOfInterval = limits.second-limits.first+1; //+1 to make sure we do not divide by 0

    for (auto c = 0 ; c < partition.size() ; c++){

        if (newCoverage.find(partition[c]) == newCoverage.end()){
            newCoverage[partition[c]] = 0;
        }

        newCoverage[partition[c]] += max(0.0, double(limits.second-limits.first)/lengthOfInterval );

    }

    //now scale all the coverages to obtain exactly the original coverage
    // if (originalDepth != -1){ //that would mean we do not know anything about the original depth

    //     double totalCoverage = 0;
    //     for (auto cov : newCoverage){
    //         if (cov.first != -1){
    //             totalCoverage += cov.second;
    //         }
    //     }
        
    //     if (totalCoverage != 0){
    //         for (auto cov : newCoverage){
    //             newCoverage[cov.first] = cov.second * originalDepth/totalCoverage; DEBUG
    //         }
    //     }

    // }

    return newCoverage;

}

/**
 * @brief Creates the GAF corresponding to the mapping of the reads on the new GFA
 * 
 * @param allreads vector of all reads (including backbone reads which can be contigs)
 * @param backbone_reads vector of all the indices of the backbone reads in allreads
 * @param allLinks vector of all links of the GFA
 * @param allOverlaps vector of all overlaps between backbone reads and normal reads
 * @param partitions contains all the conclusions of the separate_reads algorithm
 * @param outputGAF name of the output file
 */
typedef std::tuple<int, vector<pair<string, bool>>, long int> Path; //a path is a starting position on a read, a list of contigs and their orientation relative to the read, and the index of the contig on which it aligns
void output_GAF(
    std::vector <Read> &allreads, 
    std::vector<unsigned long int> &backbone_reads, 
    std::vector<Link> &allLinks, 
    std::vector <Overlap> &allOverlaps,
    std::unordered_map<unsigned long int ,std::vector< std::pair<std::pair<int,int>, std::vector<int>  > >> &partitions,
    std::string outputGAF){

    vector<vector<Path>> readPaths (allreads.size()); //to each read we associate a path on the graph

    int max_backbone = backbone_reads.size(); 
    for (int b = 0 ; b < max_backbone ; b++){

        // cout << "here are the intervals of the partioin" << endl;
        // for (auto i : partitions[backbone_reads[b]]){
        //     cout << i.first.first << " " << i.first.second << " : ";
        //     for (auto j : i.second){
        //         cout << j << " ";
        //     }
        //     cout << endl;
        // }

        long int backbone = backbone_reads[b];

        if (partitions.find(backbone) != partitions.end() && partitions[backbone].size() > 0){
            for (int n = 0 ; n < allreads[backbone].neighbors_.size() ; n++){

                auto ov = allOverlaps[allreads[backbone].neighbors_[n]];

                long int read;
                long int end;
                int start = -1;

                if (ov.sequence1 != backbone){
                    read = ov.sequence1;
                    start = ov.position_1_1;
                    end = ov.position_1_2;
                }
                else{
                    read = ov.sequence2;
                    start = min(ov.position_2_1, ov.position_2_2);
                    end = max(ov.position_2_1, ov.position_2_2);
                }

                //go through all the intervals and see through which version this read passes
                vector<pair<string, bool>> sequence_of_traversed_contigs; //string corresponds to the name of the contig, bool is true if the contig is traversed in the same orientation as the read
                short stop = 0;
                bool firsthere = false;
                bool lasthere = false;
                int inter = 0;
                for (auto interval : partitions[backbone]){

                    if (interval.second[n] > -1 && stop < 2){
                        // if (allreads[read].name == ">0_read4" || allreads[read].name.substr(0,6) == "@fa270"){
                        //     // cout << "read " << allreads[read].name << " passes input_output yyu through " << interval.first.first << " " << interval.first.second << " " << interval.second[n] << endl;
                        //     // for (auto i : interval.second.first){
                        //     //     if (i != -2){
                        //     //         cout << i << " ";
                        //     //     }
                        //     // }
                        //     // cout << endl;
                        // }
                        sequence_of_traversed_contigs.push_back(make_pair(allreads[backbone].name+"_"+std::to_string(interval.first.first)+"_"+std::to_string(interval.second[n])
                            , ov.strand));
                        if (inter == 0){
                            firsthere = true;
                        }
                        stop = 1;

                        // if (stop){ //you have one read that was lost just before !
                        //     cout << "WHOUOU: revival here of reads " << ov.sequence1 << endl;
                        // }
                    }
                    else if (stop == 1){
                        stop = 2;
                    }
                    inter++;
                }
                //last contig
                if (stop < 2){
                    lasthere = true;
                    int right = partitions[backbone][partitions[backbone].size()-1].first.second+1;
                    sequence_of_traversed_contigs.push_back(make_pair(allreads[backbone].name+"_"+std::to_string(right)+"_0"
                            , ov.strand));
                }

                if (!ov.strand){ //then mirror the vector
                    std::reverse(sequence_of_traversed_contigs.begin(), sequence_of_traversed_contigs.end());
                }

                if (((ov.strand && !lasthere) || (!ov.strand && !firsthere)) && ((ov.strand && !firsthere) || (!ov.strand && !lasthere))){ //mark if the read does not extend to either end
                    sequence_of_traversed_contigs.push_back(make_pair("&", ov.strand));
                }
                else if ((ov.strand && !lasthere) || (!ov.strand && !firsthere)){ //mark if the read does not extend to the end
                    sequence_of_traversed_contigs.push_back(make_pair("+", ov.strand));
                }
                else if ((ov.strand && !firsthere) || (!ov.strand && !lasthere)){ //mark if the read does not extend to the beginning
                    sequence_of_traversed_contigs.push_back(make_pair("-", ov.strand));
                }
                
                if (sequence_of_traversed_contigs.size()>0){ //this should almost always be true, but it's still safer to test
                    Path path = make_tuple(start,sequence_of_traversed_contigs, backbone);
                    // if (allreads[read].name.substr(0,6) == "@d5282"){
                    //     cout << "d5282 is a readdd, path :: "<< endl;
                    //     for (auto p : get<1>(path)){
                    //         cout << p.first << " " << p.second << endl;
                    //     }

                    // }
                    readPaths[read].push_back(path);
                }
                // if ("edge_4" == allreads[backbone_reads[b]].name){// && allreads[read].name.substr(0,19) == ">SRR14289618.827569"){
                //     cout << "on backbone qdsfj is aligned " << allreads[read].name.substr(0, 19) << " " <<  << endl;
                // }
            }
        }
        else{
            for (int n = 0 ; n < allreads[backbone].neighbors_.size() ; n++){
                auto ov = allOverlaps[allreads[backbone].neighbors_[n]];
                long int read;
                int start = -1;
                int end = -1;

                bool lasthere = false;
                bool firsthere = false;
                if (ov.sequence1 != backbone){
                    read = ov.sequence1;
                    start = ov.position_1_1;
                    end = ov.position_1_2;
                }
                else{
                    read = ov.sequence2;
                    start = ov.position_2_1;
                    end = ov.position_2_2;
                }

                if (start > 100){
                    firsthere = true;
                }
                if (end < allreads[read].size()-100){
                    lasthere = true;
                }
                
                vector<pair<string, bool>> v = {make_pair(allreads[backbone].name, ov.strand)};
                if (((ov.strand && !lasthere) || (!ov.strand && !firsthere)) && ((ov.strand && !firsthere) || (!ov.strand && !lasthere))){ //mark if the read does not extend to either end
                    v.push_back(make_pair("&", ov.strand));
                }
                else if ((ov.strand && !lasthere) || (!ov.strand && !firsthere)){ //mark if the read does not extend to the end
                    v.push_back(make_pair("+", ov.strand));
                }
                else if ((ov.strand && !firsthere) || (!ov.strand && !lasthere)){ //mark if the read does not extend to the beginning
                    v.push_back(make_pair("-", ov.strand));
                }
                Path contigpath = make_tuple(start,v, backbone);
                readPaths[read].push_back(contigpath);

                // cout << "bbb ds " << allreads[backbone_reads[b]].name << endl;
                // if ("edge_4" == allreads[backbone_reads[b]].name){// && allreads[read].name.substr(0,19) == ">SRR14289618.827569"){
                //     cout << "on backbone qdsfj is aligned " << allreads[read].name.substr(0, 19) << endl;
                // }
            }
        }
    }


    //now merge the paths that were on different contigs
    for (auto r = 0 ; r < readPaths.size() ; r++){

        if (readPaths[r].size() > 0){

            std::sort(readPaths[r].begin(), readPaths[r].end(),[] (const auto &x, const auto &y) { return get<0>(x) < get<0>(y); }); //gets the list sorted on first element of pair, i.e. position of contig on read
            
            // if (allreads[r].name.substr(0,19) == ">SRR14289618.827569"){

            //     cout << "on backbone 228 is feed aligned " << allreads[r].name.substr(0, 19) << endl;
            //     for (auto p = 0 ; p < readPaths[r].size() ; p++){
            //         Path path = readPaths[r][p];
            //         cout << "path " << p << " : ";
            //         auto contigs = get<1>(path);
            //         for (auto c : contigs){
            //             cout << c.first << " ";
            //         }
            //         cout << ", beginning on read : " << get<0>(path) << endl;
            //     }
            //     // exit(1);
            // }

            vector<Path> mergedPaths;
            Path currentPath = readPaths[r][0];
            //check if each path can be merged with next path
            for (auto p = 0 ; p<readPaths[r].size()-1 ; p++){

                long int contig = get<2> (currentPath);
                bool orientation = get<1>(currentPath)[get<1>(currentPath).size()-1].second;
                long int nextContig = get<2> (readPaths[r][p+1]);

                if (contig != nextContig){

                    vector<size_t> links;
                    if (orientation){
                        links = allreads[contig].get_links_right();
                    }
                    else{
                        links = allreads[contig].get_links_left();
                    }


                    bool merge = false;
                    for (auto li : links){
                        Link l = allLinks[li];
                        if (l.neighbor1 == nextContig || l.neighbor2 == nextContig){ //then merge
                            if ((l.end1==l.end2 && get<1>(readPaths[r][p+1])[0].second != orientation) 
                                || (l.end1!=l.end2 && get<1>(readPaths[r][p+1])[0].second == orientation)){
                                merge = true;
                            }
                        }
                    }

                    //if & in name, there is a cut there
                    char lastchar = get<1>(currentPath)[get<1>(currentPath).size()-1].first[get<1>(currentPath)[get<1>(currentPath).size()-1].first.size()-1];
                    char firstnextchar = get<1>(readPaths[r][p+1])[get<1>(readPaths[r][p+1]).size()-1].first[get<1>(readPaths[r][p+1])[get<1>(readPaths[r][p+1]).size()-1].first.size()-1];
                    if (lastchar == '&' || lastchar == '+' || firstnextchar == '-'){
                        merge = false;
                    }
                    if (lastchar == '&' || lastchar == '+' || lastchar == '-'){
                        get<1>(currentPath).erase(get<1>(currentPath).end()-1);
                    }

                    if (merge){
                        // if (get<1>(currentPath)[0].first.substr(0,12) == "edge_235@0@0" || get<1>(currentPath)[0].first.substr(0,12) == "edge_510@0@0"){
                        //     cout << "current path : " << endl;
                        //     for (auto p : get<1>(currentPath)){
                        //         cout << p.first << " " << p.second << endl;
                        //     }
                        //     cout << "merging 235 with path : in read " << r << endl;
                        //     for (auto p : get<1>(readPaths[r][p+1])){
                        //         cout << p.first << " " << p.second << endl;
                        //     }
                        // }
                        get<1>(currentPath).insert(get<1>(currentPath).end(), get<1> (readPaths[r][p+1]).begin(), get<1> (readPaths[r][p+1]).end());
                        get<2>(currentPath) = get<2> (readPaths[r][p+1]);
                    }
                    else{
                        mergedPaths.push_back(currentPath);
                        currentPath = readPaths[r][p+1];
                    }

                }
                else{
                    char lastchar = get<1>(currentPath)[get<1>(currentPath).size()-1].first[get<1>(currentPath)[get<1>(currentPath).size()-1].first.size()-1];
                    if (lastchar == '&' || lastchar == '+' || lastchar == '-'){
                        get<1>(currentPath).erase(get<1>(currentPath).end()-1);
                    }
                    mergedPaths.push_back(currentPath);
                    currentPath = readPaths[r][p+1];
                }
            }
            //if & in name, delete it
            char lastchar = get<1>(currentPath)[get<1>(currentPath).size()-1].first[get<1>(currentPath)[get<1>(currentPath).size()-1].first.size()-1];
            if (lastchar == '&' || lastchar == '+' || lastchar == '-'){
                get<1>(currentPath).erase(get<1>(currentPath).end()-1);
            }

            mergedPaths.push_back(currentPath);

            readPaths[r] = mergedPaths;
        }

    }

    //now the paths have been determined, output the file
    ofstream out(outputGAF);
    for (auto p = 0 ; p < readPaths.size() ; p++){
        for (Path path : readPaths[p]){
            if (get<1>(path).size() > 0){
                //output the path
                out << allreads[p].name << "\t-1\t"<< get<0>(path) <<"\t-1\t+\t";
                for (auto contig : get<1>(path)){
                    if (contig.second){
                        out << ">";
                    }
                    else{
                        out << "<";
                    }
                    out << contig.first;
                }
                out << "\t-1\t-1\t-1\t-1\t-1\t255\n";
            }
        }
    }

}

/**
 * @brief Merges the intervals that can be easily merged to reduce the number of intervals
 * 
 * @param partitions 
 */
void merge_intervals(std::unordered_map<unsigned long int ,std::vector< std::pair<std::pair<int,int>, std::vector<int>  > >> &partitions){

    //create a new partitions
    std::unordered_map<unsigned long int ,std::vector< std::pair<std::pair<int,int>, std::vector<int>  > >> new_partitions;

    // #pragma omp parallel for
    for (auto contig : partitions){
        
        vector <pair <pair<int,int>, vector<int>>> new_intervals;
        if (contig.second.size() > 0){

            vector <int> group = contig.second[0].second;
            int coordinate_start = contig.second[0].first.first;
            int coordinate_end = contig.second[0].first.second;
            for (int interval = 1 ; interval < contig.second.size() ; interval ++){
                //check if this interval can be merged with the previous one
                vector<int> groupThere = contig.second[interval].second;

                unordered_map<int, set<int>> stitchLeft = stitch(group, groupThere, contig.second[interval].first.first);
                unordered_map <int,set<int>> stitches;
                for (auto s : stitchLeft){
                    stitches[s.first] = s.second;
                }

                //make sure all contigs on the left and right are stitched
                set<int> all_contigs_left;
                for (int a : group){
                    all_contigs_left.emplace(a);
                }
                all_contigs_left.erase(-1);
                all_contigs_left.erase(-2);

                set<int> all_contigs_right;
                for (int a : groupThere){
                    all_contigs_right.emplace(a);
                }
                all_contigs_right.erase(-1);
                all_contigs_right.erase(-2);

                for (auto s : stitchLeft){
                    if (s.second.size() == 0){
                        stitchLeft[s.first] = all_contigs_left;
                    }
                }

                //check if all contigs on the left of the junction are stitched
                set <int> stitchedContigs;
                for (auto s : stitches){
                    for (int neighbor : s.second){
                        stitchedContigs.emplace(neighbor);
                    }
                }
                for (auto contig : all_contigs_left){
                    if (stitchedContigs.find(contig) == stitchedContigs.end()){
                        for (auto s : stitchLeft){
                            stitches[s.first].emplace(contig);
                        }
                    }
                }

                //check if all the stitches are trivial
                bool trivial = true;
                unordered_map<int, int> conversion;
                set<int> alreadySeen;
                for (auto s : stitches){
                    if (s.second.size() > 1){
                        trivial = false;
                    }
                    else{
                        if (alreadySeen.find(*s.second.begin()) != alreadySeen.end()){
                            trivial = false;
                        }
                        else{
                            alreadySeen.emplace(*s.second.begin());
                        }
                        conversion[*s.second.begin()] = s.first;
                    }
                }
                if (alreadySeen.size() < all_contigs_left.size() || all_contigs_left.size() != all_contigs_right.size()){
                    trivial = false;
                }

                if (!trivial){
                    new_intervals.push_back(make_pair(make_pair(coordinate_start, coordinate_end), group));
                    group = groupThere;
                    coordinate_start = contig.second[interval].first.first;
                    coordinate_end = contig.second[interval].first.second;
                }
                else{
                    coordinate_end = contig.second[interval].first.second;
                    for (auto read = 0 ; read < group.size() ; read  ++){
                        if (group[read] < 0 && groupThere[read] > -1){
                            group[read] = conversion[groupThere[read]];
                        }
                    }
                }

            }
            new_intervals.push_back(make_pair(make_pair(coordinate_start, coordinate_end), group));
        }
        else {
            new_intervals = contig.second;
        }
        new_partitions[contig.first] = new_intervals;
    }
    partitions = new_partitions;
}


int main(int argc, char *argv[])
{
    //parse the command line arguments
    if (argc != 19){
        std::cout << "Usage: ./create_new_contigs <original_assembly> <reads_file> <error_rate> <gro_file> <sam_file> "
                <<"<tmpfolder> <num_threads> <technology> <output_graph> <output_gaf> <polisher> <polish_everything> <path_to_minimap> <path-to-racon> <path-to-medaka> <path-to-samtools> "
                << "<path-to-python> <debug>" << std::endl;
        cout << argc << endl;
        return 1;
    }
    string original_assembly = argv[1];
    string reads_file = argv[2];
    float error_rate = stof(argv[3]);
    string split_file = argv[4];
    string sam_file = argv[5];
    string tmpFolder = argv[6];
    int num_threads = stoi(argv[7]);
    string technology = argv[8];
    string output_graph = argv[9];
    string outputGAF = argv[10];
    string polisher = argv[11];
    bool polish = stoi(argv[12]);
    string MINIMAP = argv[13];
    string RACON = argv[14];
    string MEDAKA = argv[15];
    string SAMTOOLS = argv[16];
    string path_to_python = argv[17];
    bool DEBUG = stoi(argv[18]);


    string argv0 = argv[0];
    //strip the '/build/create_new_contigs' from the end of argv0 to obtain the path to the src folder
    string path_to_src = argv0.substr(0,argv0.size()-25);

    vector <Link> allLinks;
    std::vector <Overlap> allOverlaps;
    std::vector <Read> allreads; 
    robin_hood::unordered_map<std::string, unsigned long int> indices;
    vector<unsigned long int> backbone_reads;

    parse_reads(reads_file, allreads, indices);
    parse_assembly(original_assembly, allreads, indices, backbone_reads, allLinks);
    parse_SAM(sam_file, allOverlaps, allreads, indices);

    //now parse the split file
    std::unordered_map<unsigned long int ,std::vector< std::pair<std::pair<int,int>, std::vector<int> > > > partitions;
    parse_split_file(split_file, allreads, allOverlaps, partitions);

    //first merge the intervals that can be merged
    merge_intervals(partitions);

    cout << " - Creating the .gaf file describing how the reads align on the new contigs" << endl;
    output_GAF(allreads, backbone_reads, allLinks, allOverlaps, partitions, outputGAF);

    cout << " - Creating the new contigs" << endl;
    modify_GFA(reads_file, allreads, backbone_reads, allOverlaps, partitions, allLinks, num_threads, 
        tmpFolder, error_rate, polisher, polish, technology, MINIMAP, RACON, MEDAKA, SAMTOOLS, path_to_python, path_to_src, DEBUG);

    output_GFA(allreads, backbone_reads, output_graph, allLinks);

    return 0;
}











