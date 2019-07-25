package com.LeeLab.runmicrofluidicsexperiments;

import android.support.v7.widget.RecyclerView;
import android.view.ViewGroup;
import android.widget.TextView;

/*
    Practicing with RecyclerView
    May be used in the final app
    Way to efficiently list things
    that can be scrolled on
 */

class AlanAdapter extends RecyclerView.Adapter<AlanAdapter.MyViewHolder> {

    private String[] placeholderDataset;

    public static class MyViewHolder extends RecyclerView.ViewHolder {
        public TextView tv;
        public MyViewHolder(TextView v){
            super(v);
            tv = v;
        }
    }

    public AlanAdapter(String[] pDataset){
        placeholderDataset = pDataset;
    }

    @Override
    public AlanAdapter.MyViewHolder onCreateViewHolder(ViewGroup parent, int viewType){
        //TextView v = LayoutInflater.from(parent.getContext()).inflate(R.layout., parent, false);
        return null;
    }

    @Override
    public void onBindViewHolder(MyViewHolder holder, int position){
        holder.tv.setText(placeholderDataset[position]);
    }

    //Return number of items in your RecycleView
    @Override
    public int getItemCount(){
        return placeholderDataset.length;
    }
}
