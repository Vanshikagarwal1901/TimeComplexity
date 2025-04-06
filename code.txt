//Given an usnorted array of integers, design an algorithm and implement it using a program to sort an array of elements by partitioning the array into two subarrays based on a pivot element such that one of the subarray holds values smaller than the pivot element while another subarray holds values greater than the pivot element. Pivot element should be selected randomly from the array. Your program should also find number of comparisons and swaps required for sorting the array.
#include<bits/stdc++.h>
using namespace std;

int partition(vector<int>& arr, int low, int high, int& comparisons, int& swaps) {
    int pivotIndex = low + rand() % (high - low + 1);
    int pivot = arr[pivotIndex];
    
    // Move pivot to the end
    swap(arr[pivotIndex], arr[high]);
    swaps++;

    int i = low - 1;
    for (int j = low; j < high; j++) {
        comparisons++;
        if (arr[j] < pivot) {
            i++;
            swap(arr[i], arr[j]);
            swaps++;
        }
    }
    // Move pivot to its final place
    swap(arr[i + 1], arr[high]);
    swaps++;
    return i + 1; // Return the final position of the pivot
}

void quicksort(vector<int>& arr, int low, int high, int& comparisons, int& swaps) {
    if (low < high) {
        int pi = partition(arr, low, high, comparisons, swaps);
        quicksort(arr, low, pi - 1, comparisons, swaps);
        quicksort(arr, pi + 1, high, comparisons, swaps);
    }
}

int main(){
    srand(time(0)); // Seed for random number generation
    int t;
    cout<<"Abhilakshya Bhatt\nRoll Number: 03\nUniversity Roll Number: 2023975\n";
    cout<<"Enter the number of test cases: ";
    cin>>t;
    while(t--){
        int n;
        cout<<"Enter the size of array: ";
        cin>>n;
        vector<int> arr(n);
        cout<<"Enter the elements of array: ";
        for(int i = 0; i < n; i++){
            cin>>arr[i];
        }
        int comparisons = 0, swaps = 0;
        quicksort(arr, 0, n - 1, comparisons, swaps);
        cout<<"Sorted array: ";
        for(int i = 0; i < n; i++){
            cout<<arr[i]<<" ";
        }
        cout<<"\nComparisons: "<<comparisons<<"\nSwaps: "<<swaps<<endl;
    }

    return 0;
}