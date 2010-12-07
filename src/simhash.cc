#include "./simhash.hpp"

using namespace std;
using namespace std::tr1;

//デリミタcが存在しない場合、sそのものを返す
vector<string> SimHash::split_string(const string& s, const string& c){
  vector<string> ret;
  for(int i = 0, n = 0; i <= s.length(); i = n + 1){
    n = s.find_first_of(c, i);
    if(n == string::npos) n = s.length();
    string tmp = s.substr(i, n-i);
    ret.push_back(tmp);
  }
  return ret;
}

void SimHash::set_one_data(const string& str){
  //str => d_id \s param_id : param_score
  vector<string> d = split_string(str, " ");
  
  //debug mode
  if(d[0] == str){
    if(debug_flag){
      cout << "Skip this line. "
	   << "Invalid format : " << str << endl;
    }
    return;
  }
  
  unint d_id = atoi(d[0].c_str());
  for(vector<string>::iterator i = d.begin() + 1; i != d.end(); ++i){
    vector<string> param = split_string(*i, ":");

    //debug mode
    if(param[0] == *i){
      if(debug_flag){
	cout << "Skip this line. " << str << endl;;
	cout << "Invalid format : " << param[0] << endl;
      }
      return;
    }
    unint f_id = atoi( param[0].c_str() );
    feature_table[d_id].push_back( make_pair(f_id, atof(param[1].c_str())) );
  }
}

void SimHash::set_data_from_file(char* input_file_name){
  ifstream ifs;
  string tmp;
  vector<string> ret_str;
  this->input_file_name = input_file_name;
  ifs.open(this->input_file_name, ios::in);
  while(ifs && getline(ifs, tmp)){
    set_one_data(tmp);
  }
  ifs.close();
}

void SimHash::set_hash_table_from_line(const string& str){
  vector<string> words;
  stringstream ss(str);
  string elem;
  while (ss >> elem){
    words.push_back(elem);
  }
  hash_table.push_back(make_pair(atoi(words[0].c_str()), atoi(words[1].c_str())));
}

void SimHash::set_hash_table_from_file(char* input_file_name){
  ifstream ifs;
  string tmp;

  vector<string> ret_str;
  ifs.open(input_file_name, ios::in);
  while(ifs && getline(ifs, tmp)){
    set_hash_table_from_line(tmp);
  }
  ifs.close();
}

void SimHash::initialize(){
  set_debug_flag(false);
  srand(time(0));
  input_file_name = NULL;
  q_id = PRIME;
}

void SimHash::save_hash_table_to_file(){
  ostringstream oss;
  vector<string> ret = split_string(input_file_name, ".");
  if(ret.size() > 1){
    for(int i = 0; i < ret.size() - 1; ++i){
      oss << ret[i];
    }
  }else{
    oss << ret[0];
  }
  oss << ".hash";
  
  ofstream ofs;
  ofs.open((oss.str()).c_str());
  for(vector<pair<unint, unint> >::iterator i = hash_table.begin(); i != hash_table.end(); ++i){
    ofs << (*i).first << " " << (*i).second << endl;
  }
  ofs.close();
}


//0 以上 limit 未満の乱数取得
unint SimHash::get_random(unint limit = 0){
  if(limit == 0){
    return rand() / (RAND_MAX / (PRIME));
  }else{
    return rand() / (RAND_MAX / (limit));
  }
}

//v のビット列をシャッフル(を近似)
unint SimHash::bit_shuffle(unint v, unint a, unint b, unint p){
  unint this_p = p;
  if(p == 0){this_p = PRIME;}
  return (a*v + b) % this_p;
}

unint SimHash::convert_data_to_hash(const vector<pair<unint, double> >& data){
  //この v[i] の正負が最終的なハッシュ値におけるiビット目の1/0になる
  //とりあえず uint32_t で決め打ちしてるので要素数32で初期化
  int v[sizeof(unint)*8] = {0};
  for(vector<pair<unint, double> >::const_iterator f = data.begin(); f != data.end(); ++f){
    //特徴の id をそのまま特徴に対するハッシュとして使う
    //あとはそれから1ビットずつ取得
    unint h = (*f).first;
    for(int i = 0; i < sizeof(unint)*8; ++i){
      if(h & 1){
	v[i] += (*f).second;
      }else{
	v[i] -= (*f).second;
      }
      h = h >> 1;
    }
  }

  unint h_value = 0;
  //vの後ろからビットを左シフトで押しこんでいく
  for(int i = sizeof(unint)*8 - 1; i >= 0; --i){
    if(v[i] >= 0){
      //左シフトさせて一番下のビットを立てる
      h_value = (h_value << 1) | 1;
    }else{
      //左シフトさせるだけ
      h_value = (h_value << 1);
    }
  }
  return h_value;
}

void SimHash::set_hash_table_from_feature_table(){
  unordered_map<unint, vector<pair<unint, double> > >::iterator i;
  for(i = feature_table.begin(); i != feature_table.end(); ++i){
    unint d_id = i->first;
    hash_table.push_back(make_pair(d_id, convert_data_to_hash(i->second)));
  }
}

unint SimHash::set_query_to_hash_table(char* input_query_name){
  unint d_id = PRIME;
  ifstream ifs;
  string tmp;

  this->input_query_name = input_query_name;

  vector<pair<unint, double> > features;
  //tmp => param_id : param_score \s param_id : param_score...
  ifs.open(input_query_name, ios::in);
  while(ifs && getline(ifs, tmp)){
    vector<string> d = split_string(tmp, " ");
    
    if(d[0] == tmp){
      cout << "Skip this line. "
	   << "Invalid format : " << tmp << endl;
      exit(1);
    }

    for(vector<string>::iterator i = d.begin(); i != d.end(); ++i){
      vector<string> param = split_string(*i, ":");
      
      if(param[0] == *i){
	cout << "Skip this line. " << tmp << endl;;
	cout << "Invalid format : " << param[0] << endl;
	exit(1);
      }
      unint f_id = atoi( param[0].c_str() );
      double f_value = atof( param[1].c_str() );
      //set feature and hash-value
      query_feature[f_id] = f_value;
      features.push_back(make_pair(f_id, f_value));
    }
    
  }
  ifs.close();

  query_hash = convert_data_to_hash(features);
  hash_table.push_back(make_pair(d_id, query_hash));
  return query_hash;
}

void SimHash::hash_table_bit_shuffle(){
  unint a = get_random();
  unint b = get_random();
  vector<pair<unint, unint> >::iterator i;
  for(i = hash_table.begin(); i != hash_table.end(); ++i){
    (*i).second = bit_shuffle((*i).second, a, b);
    //クエリのハッシュを更新しておく
    if((*i).first == PRIME){
      query_hash = (*i).second;
    }

  }
}

void SimHash::hash_table_sort(){
  sort(hash_table.begin(), hash_table.end(), pairless());
}

//クエリからプラスマイナスb個(最大2b個)取得してそのd_idを返す
void SimHash::search_b_nearest_data(unint b){
  vector<pair<unint, unint> >::iterator query;
  query = find(hash_table.begin(), hash_table.end(), make_pair(q_id, query_hash));

  if(query != hash_table.end()){
    //クエリから後ろに最大b個
    for(int i = 1; (i <= b) && (query + i != hash_table.end()) ; ++i){
      near_ids.push_back((*(query+i)).first);
    }

    //クエリから前に最大b個
    for(int i = 1; (i <= b); ++i){
      near_ids.push_back((*(query-i)).first);
      if(query-i == hash_table.begin()){
	break;
      }
    }
  }
}

double SimHash::calculate_cosine_distance(unint q_id, unint d_id){
  double norm_q = 0.0, norm_d = 0.0, inner = 0.0;
  
  vector<pair<unint, double> > d_feature = feature_table[d_id];
  for(vector<pair<unint, double> >::iterator i = d_feature.begin(); i != d_feature.end(); ++i){
    norm_d += pow((*i).second, 2.0);
    if(query_feature.end() != query_feature.find((*i).first)){
      inner += (*i).second * query_feature[(*i).first];
    }
  }

  for(unordered_map<unint, double>::iterator i = query_feature.begin(); i != query_feature.end(); ++i){
    norm_q += pow(i->second, 2.0);
  }

  if(norm_q > 0 && norm_d > 0){
    return inner / (pow(norm_q, 0.5) * pow(norm_d, 0.5));
  }else{
    return 0.0;
  }
}


void SimHash::calc_b_nearest_cosine_distance(unint b){
  //near_ids のuniqを取っておく
  sort(near_ids.begin(), near_ids.end());
  near_ids.erase(unique(near_ids.begin(), near_ids.end()), near_ids.end());

  for(vector<unint>::iterator id = near_ids.begin(); id != near_ids.end(); ++id){
    near_cosines.push_back(make_pair(*id, calculate_cosine_distance(q_id, *id)));
  }

  sort(near_cosines.begin(), near_cosines.end(), pairlessdouble());
}

void SimHash::save_near_cosines_to_file(int limit){
  ostringstream oss;
  oss << input_query_name << ".out";
  ofstream ofs;
  ofs.open((oss.str()).c_str());
  int count = 1;
  for(vector<pair<unint, double> >::iterator i = near_cosines.begin(); i != near_cosines.end(); ++i){
    ofs << (*i).first << "," << (*i).second << endl;
    count++;
    if(count >= limit){
      break;
    }
  }
  ofs.close();
}
