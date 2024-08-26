import { useEffect, useState } from 'react';
import { logoutSession } from '../redux/apiservice';
import { useDispatch } from 'react-redux';

const useNavigationPrompt = (shouldPrompt: boolean) => {
  const [showPrompt, setShowPrompt] = useState(false);
  const dispatch = useDispatch();

  useEffect(() => {
    const handleBeforeUnload = (event: BeforeUnloadEvent) => {
      if (shouldPrompt) {
        const confirmationMessage = 'Are you sure you want to leave?';
        event.preventDefault();
        setShowPrompt(true); // Show custom prompt
        return confirmationMessage; // Gecko, WebKit, Chrome <34
      }
    };

    const handleUnload = () => {
      dispatch(logoutSession('Fsfsf'));
      const navigationEntries = performance.getEntriesByType('navigation');
      if (navigationEntries.length > 0) {
        // const isReload = (navigationEntries[0] as PerformanceNavigationTiming).type === 'reload';
        // if (!isReload) {
        //   onUnload();
        // }
      }
    };

    window.addEventListener('beforeunload', handleBeforeUnload);
    window.addEventListener('unload', handleUnload);

    return () => {
      window.removeEventListener('beforeunload', handleBeforeUnload);
      window.removeEventListener('unload', handleUnload);
    };
  }, [shouldPrompt, dispatch]);

  return showPrompt;
};

export default useNavigationPrompt;
