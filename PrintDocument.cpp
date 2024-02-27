#include <algorithm>
#include <iostream>
#include <vector>
#include <limits>

using namespace std;

struct Document {
    int id;
    int rating;
}; 

void PrintDocuments(vector<Document> documents, size_t skip_start, size_t skip_finish) {
        if(documents.empty()                                                        //
         && (skip_start + documents.size()) >= numeric_limits<size_t>::max()        //
         && (skip_finish + documents.size()) >= numeric_limits<size_t>::max()) return;
    
		sort(documents.begin(), documents.end(), [](const Document& lhs, const Document& rhs) 
		{
		   return lhs.rating > rhs.rating;
		});
		
		vector<size_t> v_id;
		vector<size_t> v_rating;
		for (const auto [id, rating] : documents) {
		    v_id.push_back(id);
		    v_rating.push_back(rating);
		}
		
		if(skip_start < documents.size() && skip_finish < documents.size()) {
		    
		    for(size_t i = skip_start; i + skip_finish < documents.size(); ++i) {
		        
		    cout << "{ id = " << v_id[i] << ", rating = " << v_rating[i] << " }" << endl; 
	    	}
		}
}

int main() {
    PrintDocuments(
        {
            {100, 5},
            {101, 7},
            {102, -4},
            {103, 9},
            {104, 1}
        },
        1,
        2
    );
}
