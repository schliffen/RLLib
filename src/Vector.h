/*
 * Copyright 2013 Saminda Abeyruwan (saminda@cs.miami.edu)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Vector.h
 *
 *  Created on: Aug 18, 2012
 *      Author: sam
 */

#ifndef VECTOR_H_
#define VECTOR_H_

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <functional>
#include <numeric>
#include <cmath>
#include <cassert>
#include <cstdio>

namespace RLLib
{

// Forward declarations
template<class T> class DenseVector;
template<class T> class SparseVector;
template<class T> std::ostream& operator<<(std::ostream& out, const DenseVector<T>& that);
template<class T> std::ostream& operator<<(std::ostream& out, const SparseVector<T>& that);

/**
 * This is used in parameter representation in a given vector space for
 * Machine Learning purposes. This implementation is specialized for sparse
 * vector representation, that is much common in Reinforcement Learning.
 */
template<class T>
class Vector
{
  public:
    virtual ~Vector()
    {
    }
    virtual int dimension() const =0;
    virtual double maxNorm() const =0;
    virtual double euclideanNorm() const =0;
    virtual T* operator()() const =0; // return the data as an array
    virtual bool empty() const =0;

    virtual void persist(const std::string& f) const =0;
    virtual void resurrect(const std::string& f) =0;

  protected:
    template<class U> void write(std::ostream &o, U& value) const
    {
      char *s = (char *) &value;
      o.write(s, sizeof(value));
    }

    template<class U> void read(std::istream &i, U& value) const
    {
      char *s = (char *) &value;
      i.read(s, sizeof(value));
    }
};

template<class T>
class DenseVector: public Vector<T>
{
  protected:
    int capacity;
    T* data;

  public:
    DenseVector(const int& capacity = 1) :
        capacity(capacity), data(new T[capacity])
    {
      std::fill(data, data + capacity, 0);
    }

    virtual ~DenseVector()
    {
      delete[] data;
    }

    // Implementation details for copy constructor and operator
    DenseVector(const DenseVector<T>& that) :
        capacity(that.capacity), data(new T[that.capacity])
    {
      std::copy(that.data, that.data + that.capacity, data);
    }

    DenseVector<T>& operator=(const DenseVector<T>& that)
    {
      if (this != &that)
      {
        delete[] data; // delete old
        capacity = that.capacity;
        data = new T[capacity];
        std::copy(that.data, that.data + capacity, data);
      }
      return *this;
    }

    // dot product
    double operator*(const DenseVector<T>& that) const
    {
      assert(capacity == that.capacity);
      double tmp = 0;
      for (int i = 0; i < capacity; i++)
        tmp += data[i] * that.data[i];
      return tmp;
    }
    DenseVector<T>& operator*(const double& d)
    {
      for (T* i = data; i < data + capacity; ++i)
        *i *= d;
      return *this;
    }
    DenseVector<T>& operator+(const DenseVector<T>& that)
    {
      assert(capacity == that.capacity);
      for (int i = 0; i < capacity; i++)
        data[i] += that.data[i];
      return *this;
    }
    DenseVector<T>& operator-(const DenseVector<T>& that)
    {
      assert(capacity == that.capacity);
      for (int i = 0; i < capacity; i++)
        data[i] -= that.data[i];
      return *this;
    }

    T& operator[](const int& index) const
    {
      assert(index >= 0 && index < capacity);
      return data[index];
    }

    T& at(const int& index) const
    {
      return (*this)[index];
    }

    int dimension() const
    {
      return capacity;
    }

    bool empty() const
    {
      return (dimension() == 0);
    }

    double maxNorm() const
    {
      double maxv = capacity > 0 ? fabs(data[0]) : 0.0;
      if (capacity > 0)
      {
        for (int i = 1; i < capacity; i++)
        {
          if (fabs(data[i]) > maxv)
            maxv = fabs(data[i]);
        }
      }
      return maxv;
    }
    double euclideanNorm() const
    {
      return sqrt((*this) * (*this));
    }

    T* operator()() const
    {
      return data;
    }

    void clear()
    {
      std::fill(data, data + capacity, 0);
    }

    void set(const DenseVector<T>& that)
    {
      assert(capacity == that.capacity);
      for (int i = 0; i < capacity; i++)
        data[i] = that.data[i];
    }

    void persist(const std::string& f) const
    {
      std::ofstream of;
      of.open(f.c_str(), std::ofstream::out);
      if (of.is_open())
      {
        // write vector type (int)
        int vectorType = 0;
        Vector<T>::write(of, vectorType);
        // write data size (int)
        Vector<T>::write(of, capacity);
        // write data
        for (int j = 0; j < capacity; j++)
          Vector<T>::write(of, data[j]);
        of.close();
        std::cout << "## DenseVector persisted=" << f << std::endl;
      }
      else
        std::cerr << "ERROR! (persist) file=" << f << std::endl;
    }

    void resurrect(const std::string& f)
    {
      std::ifstream ifs;
      ifs.open(f.c_str(), std::ifstream::in);
      if (ifs.is_open())
      {
        // Read vector type;
        int vectorType;
        Vector<T>::read(ifs, vectorType);
        assert(vectorType == 0);
        // Read capacity
        int rcapacity;
        Vector<T>::read(ifs, rcapacity);
        assert(capacity == rcapacity);
        printf("vectorType=%i rcapacity=%i \n", vectorType, rcapacity);
        // Read data
        for (int j = 0; j < capacity; j++)
          Vector<T>::read(ifs, data[j]);
        ifs.close();
        std::cout << "## DenseVector persist=" << f << std::endl;
      }
      else
      {
        std::cerr << "ERROR! (resurrected) file=" << f << std::endl;
        exit(-1);
      }
    }

    template<class O> friend std::ostream& operator<<(std::ostream& out,
        const DenseVector<O>& that);

};

/**
 * @use Tile coding; traces etc, where only a handful of features are
 *      active in out of billions of features; O(M) << O(N).
 */
template<class T>
class SparseVector: public Vector<T>
{
  protected:
    int indexesPositionLength;
    int activeIndexesLength;
    int nbActive;

    int* indexesPosition;
    int* activeIndexes;
    T* values;
  public:

    SparseVector(const int& capacity = 1, const int& activeIndexesLength = 10) :
        indexesPositionLength(capacity), activeIndexesLength(activeIndexesLength), nbActive(0), indexesPosition(
            new int[indexesPositionLength]), activeIndexes(new int[activeIndexesLength]), values(
            new T[activeIndexesLength])
    {
      std::fill(indexesPosition, indexesPosition + capacity, -1);
    }

    virtual ~SparseVector()
    {
      delete[] indexesPosition;
      delete[] activeIndexes;
      delete[] values;
    }

    SparseVector(const SparseVector<T>& that) :
        indexesPositionLength(that.indexesPositionLength), activeIndexesLength(
            that.activeIndexesLength), nbActive(that.nbActive), indexesPosition(
            new int[that.indexesPositionLength]), activeIndexes(new int[that.activeIndexesLength]), values(
            new T[that.activeIndexesLength])
    {
      std::copy(that.indexesPosition, that.indexesPosition + that.indexesPositionLength,
          indexesPosition);
      std::copy(that.activeIndexes, that.activeIndexes + that.nbActive, activeIndexes);
      std::copy(that.values, that.values + that.nbActive, values);
    }
    SparseVector<T>& operator=(const SparseVector& that)
    {
      if (this != &that)
      {
        delete[] indexesPosition;
        delete[] activeIndexes;
        delete[] values;
        indexesPositionLength = that.indexesPositionLength;
        activeIndexesLength = that.activeIndexesLength;
        nbActive = that.nbActive;
        indexesPosition = new int[indexesPositionLength];
        activeIndexes = new int[activeIndexesLength];
        values = new T[activeIndexesLength];

        std::copy(that.indexesPosition, that.indexesPosition + that.indexesPositionLength,
            indexesPosition);
        std::copy(that.activeIndexes, that.activeIndexes + that.nbActive, activeIndexes);
        std::copy(that.values, that.values + that.nbActive, values);

      }
      return *this;
    }

    // bunch of helper methods
  private:
    void updateEntry(const int& index, const T& value, const int& position)
    {
      values[position] = value;
    }

    void swapEntry(const int& positionA, const int& positionB)
    {
      int indexA = activeIndexes[positionA];
      T valueA = values[positionA];
      int indexB = activeIndexes[positionB];
      T valueB = values[positionB];
      indexesPosition[indexA] = positionB;
      indexesPosition[indexB] = positionA;
      activeIndexes[positionA] = indexB;
      activeIndexes[positionB] = indexA;
      values[positionA] = valueB;
      values[positionB] = valueA;
    }

    void removeEntry(const int& position, const int& index)
    {
      swapEntry(nbActive - 1, position);
      indexesPosition[activeIndexes[nbActive - 1]] = -1;
      nbActive--;
    }

  public:
    void removeEntry(const int& index)
    {
      int position = indexesPosition[index];
      if (position != -1)
        removeEntry(position, index);
    }

    void setEntry(const int& index, const T& value)
    {
      if (value == 0)
        removeEntry(index);
      else
        setNonZeroEntry(index, value);
    }

  private:
    void allocate(int sizeRequired)
    {
      if (activeIndexesLength >= sizeRequired)
        return;
      int newCapacity = (sizeRequired * 3) / 2 + 1;
      int* newActiveIndexes = new int[newCapacity];
      T* newValues = new T[newCapacity];

      std::copy(activeIndexes, activeIndexes + activeIndexesLength, newActiveIndexes);
      std::fill(newActiveIndexes + activeIndexesLength, newActiveIndexes + newCapacity, 0);

      std::copy(values, values + activeIndexesLength, newValues);
      std::fill(newValues + activeIndexesLength, newValues + newCapacity, 0);

      activeIndexesLength = newCapacity;
      // remove old pointers
      delete[] activeIndexes;
      delete[] values;
      // set new pointers
      activeIndexes = newActiveIndexes;
      values = newValues;
    }

    void appendEntry(const int& index, const T& value)
    {
      allocate(nbActive + 1);
      activeIndexes[nbActive] = index;
      values[nbActive] = value;
      indexesPosition[index] = nbActive;
      nbActive++;
    }

  public:
    void insertLast(const T& value)
    {
      appendEntry(indexesPositionLength - 1, value);
    }

    void insertEntry(const int& index, const T& value)
    {
      appendEntry(index, value);
    }

    const T getEntry(const int& index) const
    {
      int position = indexesPosition[index];
      return position != -1 ? values[position] : T(0);
    }

  private:
    void setNonZeroEntry(const int& index, const T& value)
    {
      int position = indexesPosition[index];
      if (position != -1)
        updateEntry(index, value, position);
      else
        insertEntry(index, value);
    }

  public:
    void clear()
    {
      for (int i = 0; i < nbActive; i++)
        indexesPosition[activeIndexes[i]] = -1;
      nbActive = 0;
    }

    SparseVector<T>& addToSelf(const double& factor, const SparseVector<T>& that)
    {
      assert(dimension() == that.dimension());
      for (int position = 0; position < that.nbActive; position++)
      {
        int index = that.activeIndexes[position];
        setNonZeroEntry(index, getEntry(index) + factor * that.values[position]);
      }
      return *this;
    }

    SparseVector<T>& addToSelf(const SparseVector<T>& that)
    {
      return addToSelf(1.0, that);
    }

    SparseVector<T>& substractToSelf(const SparseVector<T>& that)
    {
      return addToSelf(-1.0, that);
    }

    SparseVector<T>& multiplyToSelf(const double& factor)
    {
      if (factor == 0)
      {
        clear();
        return *this;
      }

      for (T* position = values; position < values + nbActive; ++position)
        *position *= factor;
      return *this;
    }

    SparseVector<T>& ebeMultiplyToSelf(const SparseVector<T>& that)
    {
      assert(dimension() == that.dimension());
      int position = 0;
      while (position < nbActive)
      {
        int index = activeIndexes[position];
        T value = values[position] * that.getEntry(index);
        if (value != 0)
        {
          values[position] = value;
          ++position;
        }
        else
          removeEntry(position, index);
      }
      return *this;
    }

    SparseVector<T>& ebeDivideToSelf(const SparseVector<T>& that)
    {
      for (int position = 0; position < nbActive; position++)
      {
        int index = activeIndexes[position];
        values[position] /= that.getEntry(index); // prior check
      }
      return *this;
    }

    SparseVector<T>& ebeAddConstantToSelf(const double& value)
    {
      for (int index = 0; index < indexesPositionLength; index++)
        setNonZeroEntry(index, value + getEntry(index));
      return *this;
    }

  private:
    double dot(const SparseVector<T>& _this, const SparseVector<T>& _that) const
    {
      double tmp = 0;
      for (int position = 0; position < _this.nbActive; position++)
        tmp += _that.getEntry(_this.activeIndexes[position]) * _this.values[position];
      return tmp;
    }

  public:
    // w'* phi
    double dot(const SparseVector<T>& that) const
    {
      assert(dimension() == that.dimension());
      if (nbActive < that.nbActive)
        return dot(*this, that);
      else
        return dot(that, *this);
    }

    double sum() const
    {
      return std::accumulate(values, values + nbActive, 0.0);
    }

    // Shallow copy of that to this.
    SparseVector<T>& set(const SparseVector<T>& that)
    {
      assert(dimension() == that.dimension());
      clear();
      for (int i = 0; i < that.nbActive; i++)
        setNonZeroEntry(that.activeIndexes[i], that.values[i]);
      return *this;
    }

    SparseVector<T>& set(const SparseVector<T>& that, const int& offset)
    {
      // Dimension check is relaxed.
      clear();
      for (int i = 0; i < that.nbActive; i++)
        setNonZeroEntry(that.activeIndexes[i] + offset, that.values[i]);
      return *this;
    }

    SparseVector<T>& set(const SparseVector<T>& that, const T& value)
    {
      assert(dimension() == that.dimension());
      clear();
      for (int i = 0; i < that.nbActive; i++)
        setNonZeroEntry(that.activeIndexes[i], value);
      return *this;
    }

    SparseVector<T>& set(const T& value)
    {
      // This will set 'value' to all the elements
      clear();
      for (int index = 0; index < indexesPositionLength; index++)
        setNonZeroEntry(index, value);
      return *this;
    }

    const T* getValues() const
    {
      return values;
    }

    const int* getActiveIndexes() const
    {
      return activeIndexes;
    }

    const int* getIndexesPosition() const
    {
      return indexesPosition;
    }

    int nbActiveEntries() const
    {
      return nbActive;
    }

    int dimension() const
    {
      return indexesPositionLength;
    }

    bool empty() const
    {
      return (dimension() == 0);
    }

    double maxNorm() const
    {
      double maxv = nbActive > 0 ? fabs(values[0]) : 0.0;
      if (nbActive > 0)
      {
        for (int position = 1; position < nbActive; position++)
        {
          if (fabs(values[position]) > maxv)
            maxv = fabs(values[position]);
        }
      }
      return maxv;
    }
    double euclideanNorm() const
    {
      return sqrt(dot(*this));
    }

    T* operator()() const
    {
      return values;
    }

    void persist(const std::string& f) const
    {
      std::ofstream of;
      of.open(f.c_str(), std::ofstream::out);
      if (of.is_open())
      {
        // Write vector type (int)
        int vectorType = 1;
        Vector<T>::write(of, vectorType);
        // Write indexesPositionLength (int)
        Vector<T>::write(of, indexesPositionLength);
        // Write numActive (int)
        Vector<T>::write(of, nbActive);
        // Verbose
        printf("vectorType=%i capacity=%i nbActive=%i\n", vectorType, indexesPositionLength,
            nbActive);
        // Write active indexes
        for (int position = 0; position < nbActive; position++)
          Vector<T>::write(of, activeIndexes[position]);
        // Write active values
        for (int position = 0; position < nbActive; position++)
          Vector<T>::write(of, values[position]);
        of.close();
        std::cout << "## SparseVector persisted=" << f << std::endl;
      }
      else
        std::cerr << "ERROR! (persist) file=" << f << std::endl;
    }

    void resurrect(const std::string& f)
    {
      std::ifstream ifs;
      ifs.open(f.c_str(), std::ifstream::in);
      if (ifs.is_open())
      {
        // Read vector type;
        int vectorType;
        Vector<T>::read(ifs, vectorType);
        assert(vectorType == 1);
        // Read indexesPositionLength
        int rcapacity;
        Vector<T>::read(ifs, rcapacity);
        // Read numActive
        int rnbActive;
        Vector<T>::read(ifs, rnbActive);
        assert(indexesPositionLength == rcapacity);
        // Verbose
        printf("vectorType=%i rcapacity=%i rnbActive=%i\n", vectorType, rcapacity, rnbActive);
        // Read active indexes
        int* ractiveIndexes = new int[rnbActive];
        for (int position = 0; position < rnbActive; position++)
          Vector<T>::read(ifs, ractiveIndexes[position]);
        // Read active values
        for (int position = 0; position < rnbActive; position++)
        {
          T rvalue;
          Vector<T>::read(ifs, rvalue);
          insertEntry(ractiveIndexes[position], rvalue);
        }
        ifs.close();

        delete[] ractiveIndexes;
        std::cout << "## SparseVector resurrected=" << f << std::endl;
      }
      else
      {
        std::cerr << "ERROR! (resurrect) file=" << f << std::endl;
        exit(-1);
      }
    }

    template<class O> friend std::ostream& operator<<(std::ostream& out,
        const SparseVector<O>& that);

    // Static
    inline static void absToSelf(SparseVector<T>& that)
    {
      for (T* position = that.values; position < that.values + that.nbActive; ++position)
        *position = fabs(*position);
    }

    inline static void multiplySelfByExponential(SparseVector<T>& result, const double& factor,
        const SparseVector<T>& other, const double& min)
    {
      const int* activeIndexes = other.getActiveIndexes();
      for (int i = 0; i < other.nbActiveEntries(); i++)
      {
        int index = activeIndexes[i];
        result.setEntry(index,
            std::max(min, result.getEntry(index) * std::exp(factor * other.getEntry(index))));
      }
    }

    inline static void positiveMaxToSelf(SparseVector<T>& result, const SparseVector<T>& other)
    {
      const int* activeIndexes = other.getActiveIndexes();
      for (int i = 0; i < other.nbActiveEntries(); i++)
      {
        int index = activeIndexes[i];
        result.setEntry(index, std::max(result.getEntry(index), other.getEntry(index)));
      }
    }
};

template<class T>
class SparseVectors
{
  protected:
    typename std::vector<SparseVector<T>*>* vectors;
  public:
    typedef typename std::vector<SparseVector<T>*>::iterator iterator;
    typedef typename std::vector<SparseVector<T>*>::const_iterator const_iterator;

    SparseVectors() :
        vectors(new std::vector<SparseVector<T>*>())
    {
    }

    ~SparseVectors()
    {
      vectors->clear();
      delete vectors;
    }

    SparseVectors(const SparseVectors<T>& that) :
        vectors(new std::vector<SparseVector<T>*>())
    {
      for (typename SparseVectors<T>::iterator iter = that.begin(); iter != that.end(); ++iter)
        vectors->push_back(*iter);
    }

    SparseVectors<T>& operator=(const SparseVectors<T>& that)
    {
      if (this != that)
      {
        vectors->clear();
        for (typename SparseVectors<T>::iterator iter = that.begin(); iter != that.end(); ++iter)
          vectors->push_back(*iter);
      }
      return *this;
    }

    void push_back(SparseVector<T>* vector)
    {
      vectors->push_back(vector);
    }

    iterator begin()
    {
      return vectors->begin();
    }

    const_iterator begin() const
    {
      return vectors->begin();
    }

    iterator end()
    {
      return vectors->end();
    }

    const_iterator end() const
    {
      return vectors->end();
    }

    void clear()
    {
      for (typename SparseVectors<T>::iterator iter = begin(); iter != end(); ++iter)
        (*iter)->clear();
    }

    unsigned int dimension() const
    {
      return vectors->size();
    }

    const SparseVector<T>& operator[](const unsigned index) const
    {
      assert(index >= 0 && index < dimension());
      return *vectors->at(index);
    }

    SparseVector<T>* at(const unsigned index) const
    {
      assert(index >= 0 && index < dimension());
      return vectors->at(index);
    }

    void persist(std::string f) const
    {
      int i = 0;
      for (typename SparseVectors<T>::const_iterator iter = begin(); iter != end(); ++iter)
      {
        std::string fi(f);
        std::stringstream ss;
        ss << "." << i;
        fi.append(ss.str());
        (*iter)->persist(fi);
        ++i;
      }
    }

    void resurrect(std::string f) const
    {
      int i = 0;
      for (typename SparseVectors<T>::const_iterator iter = begin(); iter != end(); ++iter)
      {
        std::string fi(f);
        std::stringstream ss;
        ss << "." << i;
        fi.append(ss.str());
        (*iter)->resurrect(fi);
        ++i;
      }
    }

};

// Global implementations
template<class T>
std::ostream& operator<<(std::ostream& out, const DenseVector<T>& that)
{
  for (T* i = that.data; i < that.data + that.capacity; ++i)
    out << *i << " ";
  return out;
}

template<class T>
std::ostream& operator<<(std::ostream& out, const SparseVector<T>& that)
{
  out << "index=";
  for (int index = 0; index < that.indexesPositionLength; index++)
    out << that.indexesPosition[index] << " ";
  out << std::endl;

  for (int position = 0; position < that.nbActive; position++)
    out << "[p=" << position << " i=" << that.activeIndexes[position] << " v="
        << that.getEntry(that.activeIndexes[position]) << "] ";
  return out;
}

} // namespace RLLib

#endif /* VECTOR_H_ */
