import { useContext } from 'react';
import { RootStoreContext, RootStoreType } from './rootStore';

export const useStore = (): RootStoreType => {
  const store = useContext(RootStoreContext);
  if (!store) {
    throw new Error('useStore must be used within a RootStoreProvider.');
  }
  return store;
};