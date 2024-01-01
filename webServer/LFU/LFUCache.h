#include <string>
#include <unordered_map>
#include <set>
#include "../MemoryPool/MemoryPool.h"
#include <iostream>
using namespace std;
 
template<class KEY>
struct CacheKey
{
	KEY		key;
	int		count;
	CacheKey(KEY k) :key(k), count(1) {}
	void change(int x) {
		count += x;
	}
    int getcount() {
		return count;
	}
};
 
template<class KEY,class VAL>
struct CacheNode
{
	CacheKey<KEY>* cacheKey;
	VAL val;
	CacheNode(CacheKey<KEY>* ck, VAL v) :cacheKey(ck), val(v) {}
	CacheNode(KEY k, VAL v):val(v) {
		cacheKey = newElement<CacheKey<KEY>>(K);
	}
};
 
template<class KEY>
struct CMP     //仿函数，提供CacheKey的比较方式
{
	bool operator()(CacheKey<KEY> const* x, CacheKey<KEY> const* y) {
		return x->count < y->count;
	}
};


/*
LFU-Aging 对于LFU的改进版
LFU对于新数据不友好，因为新插入的数据访问次数count=1,放在尾部
如果需要淘汰就会将新数据淘汰。
所以LFU-Aging加入了平均访问次数的概念
如果节点的平均访问次数大于某个固定值x时，则将所有节点的count值减去x/2
这样可解决“缓存污染”
*/
template<class KEY, class VAL >
class LfuAging
{
public:
	LfuAging(size_t size,int maxAverage = 10) 
		:cacheSize(size), countMaxAverage(maxAverage), countCurAverage(1), countAll(0){
 
	}
	~LfuAging() {
		auto ite = cacheMap.begin();
		while (ite != cacheMap.end()) {
			delete ite->second->cacheKey;
			delete ite->second;
			++ite;
		}
		cacheMap.clear();
		cacheSet.clear();
	}
public:
	void		put(KEY key, VAL val) {
		this->check_cache_size();
		auto node = new CacheNode<KEY, VAL>(key, val);
		cacheMap[key] = node;
		cacheSet.insert(node->cacheKey);
        this->average(1);	
	}
 
	bool		get(KEY key, VAL* val) {
		if (cacheMap.count(key)) {
			auto node = this->removeFromSet(key);
			if (val)
				* val = node->val;
			node->cacheKey->change(1);
			cacheSet.insert(node->cacheKey);
			this->average(1);
			return true;
		}
		return false;
	}
 
	VAL			get(KEY key) {
		VAL v;
		this->get(key, &v);
		return v;
	}
 
	void        remove(KEY key) {
		auto node = this->removeFromSet(key);
		if (node) {
			cacheMap.erase(key);
			this->average(-node->cacheKey->getcount());
			delete node->cacheKey;
			delete node;
		}
	}
 
	bool		exist(KEY key) {
		if (cacheMap.count(key))
			return true;
		return false;
	}
 
	void		print_test() {
		auto ite = cacheSet.begin();
		while (ite != cacheSet.end()) {
			cout << "count: " << (*ite)->count << " key: " << (*ite)->key << endl;
			++ite;
		}
		cout << "======================================" << endl;
	}
private:
	CacheNode<KEY, VAL>* removeFromSet(KEY key) {
		if (cacheMap.count(key)) {
			auto node = cacheMap[key];
			auto ite = cacheSet.begin();
			while (ite != cacheSet.end()) {
				if ((*ite) == node->cacheKey) {
					cacheSet.erase(ite);
					break;
				}
				++ite;
			}
			return node;
		}
		return nullptr;
	}
 
	void	check_cache_size() {
		while (cacheMap.size() >= cacheSize) {
			auto ite = cacheSet.begin();
			cacheMap.erase((*ite)->key);
			cacheSet.erase(ite);
		}
	}
 
	void		average(int change) {
		countAll += change;
		int av = countAll / cacheMap.size();
		if (av >= countMaxAverage) {
			auto ite = cacheSet.begin();
			while (ite != cacheSet.end()) {
				(*ite)->change(-(countMaxAverage / 2));
				++ite;
			}
			countAll = countAll - ((countMaxAverage / 2) * cacheMap.size());
		}
	}
private:
	multiset<CacheKey<KEY>*, CMP<KEY>>            cacheSet;
	unordered_map<KEY, CacheNode<KEY, VAL>*>      cacheMap;
	size_t                                        cacheSize;
	int                                           countMaxAverage;
	int                                           countCurAverage;
	int                                           countAll;
};