// src/stores/rootStore.ts
import React from 'react';
import { ThemeStore, themeStore } from './themeStore';

export interface RootStoreType {
  themeStore: ThemeStore;
}

export const rootStore: RootStoreType = {
  themeStore,
};

export const RootStoreContext = React.createContext<RootStoreType>(rootStore);