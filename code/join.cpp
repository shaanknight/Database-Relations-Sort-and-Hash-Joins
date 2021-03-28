#include<bits/stdc++.h>
using namespace std;

const int TUPLES_IN_BLOCK = 100;
const int column_count = 2;

vector<string> read_tuple(string line)
{
	vector<string> tuple;
	istringstream buffer(line);
	int line_pending = line.size();
	for(int i=0;i<column_count;++i)
	{
		char char_read;
		string entry;
		while(1)
		{
			buffer.get(char_read);
			line_pending--;
			if(char_read == ' ' || line_pending == -1) break;
			entry.push_back(char_read);
		}
		tuple.push_back(entry);
	}
	return tuple;
}

// sort-merge join

bool comparator(vector<string> &tuple1,vector<string> &tuple2)
{	
	return tuple1[0] < tuple2[0];
}

void sortedsublist_write(int file_index,int chunk_index,vector<vector<string> > chunk)
{
	sort(chunk.begin(),chunk.end(),comparator);
	string intermediate_path = to_string(file_index)+"_"+to_string(chunk_index)+"_intermediate_file.txt";
	ofstream intermediate_write(intermediate_path);
	for(auto &row : chunk)
	{
		for(auto &entry : row)
			intermediate_write << entry << " ";
		intermediate_write << "\n";
	}
	intermediate_write.close();
}

int create_sorted_sublists(string path,int index,int M)
{
	ifstream input(path);
	string line;
	vector<vector<string> > chunk;
	int max_chunk_size = TUPLES_IN_BLOCK*M;
	int chunk_index = 0;
	while(getline(input, line, '\n'))
	{
		vector<string> tuple = read_tuple(line);
		if(index == 0) reverse(tuple.begin(),tuple.end());
		chunk.push_back(tuple);
		if((int)(chunk.size()) == max_chunk_size)
		{
	        chunk_index++;
	        sortedsublist_write(index,chunk_index,chunk);
	        chunk.clear();
		}
	}
	input.close();
	if(!chunk.empty())
	{
		chunk_index++;
		sortedsublist_write(index,chunk_index,chunk);
		chunk.clear();
	}
	return chunk_index;
}

struct compare_tuple
{
	bool operator()(pair<vector<string>, int> &tuple1, pair<vector<string>, int> &tuple2) 
	{
    	return tuple1.first[0] > tuple2.first[0];
	};
};

void sortjoin(string R_file,string S_file,int M)
{
	// output file path creation
	vector<string> tokens_R;
	stringstream Rpathstream(R_file);
	string token;
	while (getline(Rpathstream, token, '/')) {
		tokens_R.push_back(token);
	}
	string output_file = tokens_R.back();

	output_file += "_";
	vector<string> tokens_S;
	stringstream Spathstream(S_file);
	while (getline(Spathstream, token, '/')) {
		tokens_S.push_back(token);
	}
	output_file += tokens_S.back();

	output_file += "_join.txt";

	ofstream output(output_file);

	// check condition
	ifstream rin(R_file);
	string line;
	int r_tuples = 0;
	while(getline(rin, line, '\n'))
		r_tuples++;

	ifstream sin(S_file);
	line;
	int s_tuples = 0;
	while(getline(sin, line, '\n'))
		s_tuples++;
	
	int BR = (r_tuples + TUPLES_IN_BLOCK - 1)/TUPLES_IN_BLOCK;
	int BS = (s_tuples + TUPLES_IN_BLOCK - 1)/TUPLES_IN_BLOCK;

	if(BR + BS > M*M)
	{
		cout << "M not sufficient" << "\n";
		return;
	}

	// cout << "conditions checked" << endl;

	// open the files and creating sorted sublists
	int R_lists_cnt = create_sorted_sublists(R_file,0,M);
	int S_lists_cnt = create_sorted_sublists(S_file,1,M);

	// cout << "sorted sublists created" << endl;

	// performing merge join based on getnext
	vector<ifstream> Rstreams;
	for(int i=1;i<=R_lists_cnt;++i)
	{
		ifstream ifs(to_string(0)+"_"+to_string(i)+"_intermediate_file.txt");
    	Rstreams.push_back(std::move(ifs));
	}
	vector<ifstream> Sstreams;
	for(int i=1;i<=S_lists_cnt;++i)
	{
		ifstream ifs(to_string(1)+"_"+to_string(i)+"_intermediate_file.txt");
    	Sstreams.push_back(std::move(ifs));
	}

	priority_queue<pair<vector<string>, int>, vector<pair<vector<string>, int>>, compare_tuple> Rpq;
	priority_queue<pair<vector<string>, int>, vector<pair<vector<string>, int>>, compare_tuple> Spq;

	for(int i=1;i<=R_lists_cnt;++i)
	{
		for(int j=0;j<TUPLES_IN_BLOCK;++j)
		{
			if(getline(Rstreams[i-1], line, '\n'))
				Rpq.push(make_pair(read_tuple(line),i-1));
		}
	}

	for(int i=1;i<=S_lists_cnt;++i)
	{
		for(int j=0;j<TUPLES_IN_BLOCK;++j)
		{
			if(getline(Sstreams[i-1], line, '\n'))
				Spq.push(make_pair(read_tuple(line),i-1));
		}
	}

	// cout << "to be merged" << endl;

	while(!(Rpq.empty()) && !(Spq.empty()))
	{
		auto r_next = Rpq.top();
		auto s_next = Spq.top();
		// cout << r_next.first[0] << " " << s_next.first[0] << endl;
		if(r_next.first[0] > s_next.first[0])
		{
			Spq.pop();
			if(getline(Sstreams[s_next.second],line,'\n'))
    			Spq.push(make_pair(read_tuple(line),s_next.second));
		}
		else if(r_next.first[0] < s_next.first[0])
		{
    		Rpq.pop();
			if(getline(Rstreams[r_next.second],line,'\n'))
    			Rpq.push(make_pair(read_tuple(line),r_next.second));
		}
		else
		{
			string Y = r_next.first[0];
			vector<string> Rbuffer;
			vector<string> Sbuffer;
			while(r_next.first[0] == Y)
			{
				Rbuffer.push_back(r_next.first[1]);
				Rpq.pop();
				if(getline(Rstreams[r_next.second],line,'\n'))
	    			Rpq.push(make_pair(read_tuple(line),r_next.second));
	    		if(Rpq.empty())
	    			break;
	    		r_next = Rpq.top();
	    	}
	    	// cout << "out of R" << endl;
	    	while(s_next.first[0] == Y)
	    	{
	    		Sbuffer.push_back(s_next.first[1]);
	    		Spq.pop();
				if(getline(Sstreams[s_next.second],line,'\n'))
	    			Spq.push(make_pair(read_tuple(line),s_next.second));
	    		if(Spq.empty())
	    			break;
	    		s_next = Spq.top();
	    	}
	    	for(auto X:Rbuffer)
	    		for(auto Z:Sbuffer)
	    			output << X << " " << Y << " " << Z << "\n";
		}
	}

	while(!(Rpq.empty()))
		Rpq.pop();
	while(!(Spq.empty()))
		Spq.pop();

	// close the files
	output.close();
    for(int i=1;i<=R_lists_cnt;++i)
    {
    	Rstreams[i-1].close();
    	string intermediate_path = to_string(0)+"_"+to_string(i)+"_intermediate_file.txt";
    	char buffer[(int)intermediate_path.size()+1];
    	strcpy(buffer, intermediate_path.c_str());
    	std::remove(buffer);
    }
    for(int i=1;i<=S_lists_cnt;++i)
    {
    	Sstreams[i-1].close();
    	string intermediate_path = to_string(1)+"_"+to_string(i)+"_intermediate_file.txt";
    	char buffer[(int)intermediate_path.size()+1];
    	strcpy(buffer, intermediate_path.c_str());
    	std::remove(buffer);
    }
}

// hash join

int gethash(string str,int md)
{
	int n = str.size();
	int p = 1;
	int hash = 0;
	for(int i=1;i<=n;++i)
	{
		int v = 1ll*(str[i-1]-'a'+1)*p%md;
		hash = (hash + v)%md;
		p = 1ll*p*37%md;
	}
	return hash;
}

vector<int> create_hashed_sublists(string path,int index,int M)
{
	ifstream input(path);
	string line;
	vector<vector<string> > chunk;
	int max_chunk_size = TUPLES_IN_BLOCK;
	int chunk_index = 0;
	vector<vector<string> > bucket[M];
	vector<int> bucket_sizes(M,0);

	vector<ofstream> sublist_streams;
	for(int i=0;i<M;++i)
	{
		ofstream ofs(to_string(index)+"_"+to_string(i)+"_intermediate_file.txt");
    	sublist_streams.push_back(std::move(ofs));
	}

	while(getline(input, line, '\n'))
	{
		vector<string> tuple = read_tuple(line);
		if(index == 0) reverse(tuple.begin(),tuple.end());
		int hsh = gethash(tuple[0],M);
		bucket[hsh].push_back(tuple);
		if((int)bucket[hsh].size() == TUPLES_IN_BLOCK)
		{
			for(auto tuple:bucket[hsh])
				sublist_streams[hsh] << tuple[0] << " " << tuple[1] << "\n"; 
			bucket_sizes[hsh] += TUPLES_IN_BLOCK;
			bucket[hsh].clear();
		}
	}
	for(int i=0;i<M;++i)
	{
		if((int)bucket[i].size())
		{
			for(auto tuple:bucket[i])
				sublist_streams[i] << tuple[0] << " " << tuple[1] << "\n";
			bucket_sizes[i] += (int)bucket[i].size();
			bucket[i].clear();
		}
		sublist_streams[i].close();
	}
	return bucket_sizes;
}

void hashjoin(string R_file,string S_file,int M)
{
	// output file path creation
	vector<string> tokens_R;
	stringstream Rpathstream(R_file);
	string token;
	while (getline(Rpathstream, token, '/')) {
		tokens_R.push_back(token);
	}
	string output_file = tokens_R.back();

	output_file += "_";
	vector<string> tokens_S;
	stringstream Spathstream(S_file);
	while (getline(Spathstream, token, '/')) {
		tokens_S.push_back(token);
	}
	output_file += tokens_S.back();

	output_file += "_join.txt";

	ofstream output(output_file);

	vector<int> R_bucket_sizes = create_hashed_sublists(R_file,0,M);
	vector<int> S_bucket_sizes = create_hashed_sublists(S_file,1,M);

	// cout << "hashed sublists created" << endl;

	for(int i=0;i<M;++i)
	{
		ifstream Rstream(to_string(0)+"_"+to_string(i)+"_intermediate_file.txt");
    	ifstream Sstream(to_string(1)+"_"+to_string(i)+"_intermediate_file.txt");

    	if(min(R_bucket_sizes[i],S_bucket_sizes[i]) > M*TUPLES_IN_BLOCK)
    	{
    		cout << "M not sufficient for hash join : smaller of 2 bucket sizes > M" << "\n";
    		return;
    	}

    	if(R_bucket_sizes[i] < S_bucket_sizes[i])
    	{
    		// cout << "R < S" << endl;
    		string line;
    		map<string,int> mp;
    		vector<string> alts[M*TUPLES_IN_BLOCK+1];
    		int k = 0;
    		while(getline(Rstream,line,'\n'))
    		{
    			vector<string> tuple = read_tuple(line);
    			// cout << tuple[0] << " " << tuple[1] << endl;
    			if(!mp[tuple[0]]) mp[tuple[0]] = ++k;
    			alts[mp[tuple[0]]].push_back(tuple[1]); 
    		}
    		// cout << "R loaded" << endl;
    		while(getline(Sstream,line,'\n'))
    		{
    			vector<string> tuple = read_tuple(line);
    			// cout << tuple[0] << " " << tuple[1] << endl;
    			for(auto v:alts[mp[tuple[0]]])
    				output << v << " " << tuple[0] << " " << tuple[1] << "\n";
    		}
    	}
    	else
    	{
    		// cout << "S < R" << endl;
    		string line;
    		map<string,int> mp;
    		vector<string> alts[M*TUPLES_IN_BLOCK+1];
    		int k = 0;
    		while(getline(Sstream,line,'\n'))
    		{
    			vector<string> tuple = read_tuple(line);
    			if(!mp[tuple[0]]) mp[tuple[0]] = ++k;
    			alts[mp[tuple[0]]].push_back(tuple[1]); 
    		}
    		// cout << "S loaded" << endl;
    		while(getline(Rstream,line,'\n'))
    		{
    			vector<string> tuple = read_tuple(line);
    			for(auto v:alts[mp[tuple[0]]])
    				output << tuple[1] << " " << tuple[0] << " " << v << "\n";
    		}
    	}
    	// cout << "merged" << endl;
    	Rstream.close();
    	string intermediate_path = to_string(0)+"_"+to_string(i)+"_intermediate_file.txt";
    	char Rbuffer[(int)intermediate_path.size()+1];
    	strcpy(Rbuffer, intermediate_path.c_str());
    	std::remove(Rbuffer);

    	Sstream.close();
    	intermediate_path = to_string(1)+"_"+to_string(i)+"_intermediate_file.txt";
    	char Sbuffer[(int)intermediate_path.size()+1];
    	strcpy(Sbuffer, intermediate_path.c_str());
    	std::remove(Sbuffer);
	}

	output.close();
}

int main(int argc,char * argv[])
{
	string R_file = argv[1];
	string S_file = argv[2];
	string type = argv[3];
	int M = atoi(argv[4]);

	if(type == "sort")
		sortjoin(R_file,S_file,M);
	else
		hashjoin(R_file,S_file,M);
}

