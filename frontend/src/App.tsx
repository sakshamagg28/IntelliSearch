import { Navigate, Route, Routes } from 'react-router-dom';

import { Layout } from './components/Layout';
import { AnalyticsPage } from './pages/AnalyticsPage';
import { SearchPage } from './pages/SearchPage';

export function App() {
  return (
    <Routes>
      <Route element={<Layout />}>
        <Route index element={<SearchPage />} />
        <Route path="analytics" element={<AnalyticsPage />} />
        <Route path="*" element={<Navigate to="/" replace />} />
      </Route>
    </Routes>
  );
}
